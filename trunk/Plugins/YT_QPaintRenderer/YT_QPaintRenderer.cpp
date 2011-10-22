#include "YT_QPaintRenderer.h"
#include <assert.h>

#define RENDER_FREQ	60

Q_EXPORT_PLUGIN2(QPaintRenderer, QPaintRendererPlugin)

Host* g_Host = 0;
Host* GetHost()
{
	return g_Host;
}

RESULT QPaintRendererPlugin::Init( Host* host)
{
	g_Host = host;

	g_Host->RegisterPlugin(this, PLUGIN_RENDERER, QString("QPaint (Slow)"));

	return OK;
}


Renderer* QPaintRendererPlugin::NewRenderer(QWidget* widget, const QString& name )
{
	QPaintRenderer* renderer = new QPaintRenderer(g_Host, widget, name);

	return renderer;
}

void QPaintRendererPlugin::ReleaseRenderer( Renderer* parent )
{
	delete (QPaintRenderer*)parent;
}



QPaintRenderer::QPaintRenderer(Host* host, QWidget* widget, const QString& name) 
	: m_Host(host), QWidget(widget)
{
}


QPaintRenderer::~QPaintRenderer()
{
}

RESULT QPaintRenderer::RenderScene(FrameListPtr frames)
{
	int size = frames->size();

	m_Frames = frames;

	this->update();

	QMutexLocker locker(&m_MutexFramesRendered);
	m_FramesRendered.wait(&m_MutexFramesRendered, 1000);
	
	m_Frames.clear();

	return OK;
}

RESULT QPaintRenderer::Allocate( FramePtr& frame, FormatPtr sourceFormat )
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

	frame->Format()->SetColor(RGB24);
	
	return OK;
}

RESULT QPaintRenderer::Deallocate( FramePtr frame )
{
	QImage* image = (QImage*)frame->ExternData();

	delete image;

	frame.clear();

	return OK;
}

RESULT QPaintRenderer::GetFrame( FramePtr& frame )
{
	return OK;
}

RESULT QPaintRenderer::ReleaseFrame( FramePtr frame )
{
	return OK;
}

void QPaintRenderer::paintEvent( QPaintEvent* )
{
	FrameListPtr frames = m_Frames;
	if (frames->size())
	{
		int width = QWidget::width();
		int height = QWidget::height();
		if (width > 0 && height > 0)
		{			
			QPainter painter(this);
			QRect rcClient = rect();

			painter.setBrush(Qt::black);
			painter.drawRect(rcClient);

			for (int i=0; i<frames->size(); ++i) 
			{
				FramePtr frame = frames->at(i);
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
