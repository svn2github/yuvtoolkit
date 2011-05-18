#include "YT_QPaintRenderer.h"
#include <assert.h>

#define RENDER_FREQ	60

Q_EXPORT_PLUGIN2(YT_QPaintRenderer, YT_QPaintRendererPlugin)

YT_Host* g_Host = 0;
YT_Host* GetHost()
{
	return g_Host;
}

YT_RESULT YT_QPaintRendererPlugin::Init( YT_Host* host)
{
	g_Host = host;

	g_Host->RegisterPlugin(this, YT_PLUGIN_RENDERER, QString("QPaint (Slow)"));

	return YT_OK;
}


YT_Renderer* YT_QPaintRendererPlugin::NewRenderer(QWidget* widget, const QString& name )
{
	YT_QPaintRenderer* renderer = new YT_QPaintRenderer(g_Host, widget, name);

	return renderer;
}

void YT_QPaintRendererPlugin::ReleaseRenderer( YT_Renderer* parent )
{
	delete (YT_QPaintRenderer*)parent;
}



YT_QPaintRenderer::YT_QPaintRenderer(YT_Host* host, QWidget* widget, const QString& name) 
: m_Host(host), QWidget(widget), m_RenderList(NULL)
{
}


YT_QPaintRenderer::~YT_QPaintRenderer()
{
}

YT_RESULT YT_QPaintRenderer::RenderScene(QList<YT_Render_Frame>& frameList)
{
	m_RenderList = &frameList;

	this->update();

	QMutexLocker locker(&m_MutexFramesRendered);
	m_FramesRendered.wait(&m_MutexFramesRendered, 100);
	
	m_RenderList = NULL;

	return YT_OK;
}

YT_RESULT YT_QPaintRenderer::Allocate( YT_Frame_Ptr& frame, YT_Format_Ptr sourceFormat )
{
	QImage* image = new QImage(sourceFormat->Width(), sourceFormat->Height(), QImage::Format_RGB888);

	frame = m_Host->NewFrame();
	frame->SetFormat(*sourceFormat);

	frame->SetExternData((void*) image);
	frame->SetData(0, image->bits());
	frame->SetData(1, 0);
	frame->SetData(2, 0);
	frame->SetData(3, 0);

	// support only rgb24
	frame->Format().SetStride(0,sourceFormat->Width()*3);
	frame->Format().SetStride(1,0);
	frame->Format().SetStride(2,0);
	frame->Format().SetStride(3,0);

	frame->Format().SetColor(YT_RGB24);
	
	return YT_OK;
}

YT_RESULT YT_QPaintRenderer::Deallocate( YT_Frame_Ptr frame )
{
	QImage* image = (QImage*)frame->ExternData();

	delete image;

	m_Host->ReleaseFrame(frame);

	return YT_OK;
}

YT_RESULT YT_QPaintRenderer::GetFrame( YT_Frame_Ptr& frame )
{
	return YT_OK;
}

YT_RESULT YT_QPaintRenderer::ReleaseFrame( YT_Frame_Ptr frame )
{
	return YT_OK;
}

void YT_QPaintRenderer::OnPaintTimer()
{
	if (m_RenderList)
	{
		if (!isVisible())
		{
			show();
		}

		QRect rc = parentWidget()->rect();
		if (rc != rect())
		{
			move(rc.left(), rc.top());
			resize(rc.width(), rc.height());
		}
		
		repaint();
	}
}

void YT_QPaintRenderer::paintEvent( QPaintEvent* )
{
	QList<YT_Render_Frame>* renderList = m_RenderList;
	if (renderList)
	{
		int width = QWidget::width();
		int height = QWidget::height();
		if (width > 0 && height > 0)
		{			
			QPainter painter(this);
			QRect rcClient = rect();

			painter.setBrush(Qt::black);
			painter.drawRect(rcClient);

			for (int i=0; i<renderList->size(); ++i) 
			{
				const YT_Render_Frame& rf = renderList->at(i);

				QRect srcRect(rf.srcRect[0], rf.srcRect[1], rf.srcRect[2]-rf.srcRect[0], rf.srcRect[3]-rf.srcRect[1]);
				QRect dstRect(rf.dstRect[0], rf.dstRect[1], rf.dstRect[2]-rf.dstRect[0], rf.dstRect[3]-rf.dstRect[1]);

				painter.drawImage(dstRect, *(QImage*)(rf.frame->ExternData()), srcRect);
			}
		}

		m_RenderList = NULL;
		m_FramesRendered.wakeAll();
	}
}
