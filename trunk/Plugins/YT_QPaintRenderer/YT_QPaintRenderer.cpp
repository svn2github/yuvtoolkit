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
	: m_Host(host), QWidget(widget)
{
}


YT_QPaintRenderer::~YT_QPaintRenderer()
{
}

YT_RESULT YT_QPaintRenderer::RenderScene(YT_Frame_List frames)
{
	int size = frames.size();

	m_Frames = frames;

	this->update();

	QMutexLocker locker(&m_MutexFramesRendered);
	m_FramesRendered.wait(&m_MutexFramesRendered, 1000);
	
	m_Frames.clear();

	return YT_OK;
}

YT_RESULT YT_QPaintRenderer::Allocate( YT_Frame_Ptr& frame, YT_Format_Ptr sourceFormat )
{
	QImage* image = new QImage(sourceFormat->Width(), sourceFormat->Height(), QImage::Format_RGB888);

	frame = m_Host->NewFrame();
	frame->SetFormat(sourceFormat);

	frame->SetExternData((void*) image);
	frame->SetData(0, image->bits());
	frame->SetData(1, 0);
	frame->SetData(2, 0);
	frame->SetData(3, 0);

	// support only rgb24
	frame->Format()->SetStride(0,sourceFormat->Width()*3);
	frame->Format()->SetStride(1,0);
	frame->Format()->SetStride(2,0);
	frame->Format()->SetStride(3,0);

	frame->Format()->SetColor(YT_RGB24);
	
	return YT_OK;
}

YT_RESULT YT_QPaintRenderer::Deallocate( YT_Frame_Ptr frame )
{
	QImage* image = (QImage*)frame->ExternData();

	delete image;

	frame.clear();

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

void YT_QPaintRenderer::paintEvent( QPaintEvent* )
{
	if (m_Frames.size())
	{
		int width = QWidget::width();
		int height = QWidget::height();
		if (width > 0 && height > 0)
		{			
			QPainter painter(this);
			QRect rcClient = rect();

			painter.setBrush(Qt::black);
			painter.drawRect(rcClient);

			for (int i=0; i<m_Frames.size(); ++i) 
			{
				YT_Frame_Ptr frame = m_Frames.at(i);
				if (!frame)
				{
					continue;
				}

				const QRect srcRect = frame->Info(SRC_RECT).toRect();
				const QRect dstRect = frame->Info(DST_RECT).toRect();

				painter.drawImage(dstRect, *(QImage*)(frame->ExternData()), srcRect);
			}
		}

		m_Frames.clear();

		m_FramesRendered.wakeAll();
	}
}
