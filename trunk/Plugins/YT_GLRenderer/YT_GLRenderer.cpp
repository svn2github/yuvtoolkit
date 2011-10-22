#include "YT_GLRenderer.h"
#include <assert.h>

#define RENDER_FREQ	60

Q_EXPORT_PLUGIN2(OpenGLRenderer, OpenGLRendererPlugin)

Host* g_Host = 0;
Host* GetHost()
{
	return g_Host;
}

RESULT OpenGLRendererPlugin::Init( Host* host)
{
	g_Host = host;

	if (QGLFormat::hasOpenGL() && QGLPixelBuffer::hasOpenGLPbuffers()) 
	{
		g_Host->RegisterPlugin(this, PLUGIN_RENDERER, QString("OpenGL"));
	}

	return OK;
}


Renderer* OpenGLRendererPlugin::NewRenderer(QWidget* widget, const QString& name )
{
	OpenGLRenderer* renderer = new OpenGLRenderer(g_Host, widget, name);

	return renderer;
}

void OpenGLRendererPlugin::ReleaseRenderer( Renderer* parent )
{
	delete (OpenGLRenderer*)parent;
}



OpenGLRenderer::OpenGLRenderer(Host* host, QWidget* widget, const QString& name) 
: m_Host(host), QGLWidget(widget), m_ReadyToRender(false)
{
	setAutoBufferSwap(false); // swap buffer in rendering thread	

	m_BufferSizeChanged = true;
	m_BufferWidth = 640;
	m_BufferHeight = 480;

	QTimer *timer = new QTimer(widget);
	connect(timer, SIGNAL(timeout()), this, SLOT(OnResizeTimer()));
	timer->start(300);
}


OpenGLRenderer::~OpenGLRenderer()
{
}

RESULT OpenGLRenderer::RenderScene(FrameList frames)
{
	if (m_ReadyToRender) 
	{
		makeCurrent();

		if (m_BufferSizeChanged) {
			glViewport(0, 0, m_BufferWidth, m_BufferHeight);
			// Set up 2D view, and map topleft to 0,0
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, m_BufferWidth, m_BufferHeight, 0, 0, 1);
			glMatrixMode(GL_MODELVIEW);

			// Disable depth
			glDisable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);     // Background => dark blue
		for (int i=0; i<frames.size(); ++i) 
		{
			FramePtr frame = frames.at(i);
			if (!frame)
			{
				continue;
			}

			const QRect srcRect = frame->Info(SRC_RECT).toRect();
			const QRect dstRect = frame->Info(DST_RECT).toRect();

			QImage image = *(QImage*)(frame->ExternData());
			glRasterPos2f(dstRect.left(), dstRect.top());
			glPixelZoom(((float)(dstRect.width()))/(srcRect.width()), 
				-1.0 * ((float)(dstRect.height()))/(srcRect.height()));	
			glPixelStorei(GL_UNPACK_ROW_LENGTH, image.width());
			const uchar* bits  = image.bits() + (srcRect.top())*image.width()*3 + srcRect.left()*3;
			glDrawPixels(srcRect.width(), srcRect.height(), GL_RGB, GL_UNSIGNED_BYTE, bits);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}

		swapBuffers();
	}

	return OK;
}

RESULT OpenGLRenderer::Allocate( FramePtr& frame, FormatPtr sourceFormat )
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

RESULT OpenGLRenderer::Deallocate( FramePtr frame )
{
	QImage* image = (QImage*)frame->ExternData();

	delete image;

	frame.clear();

	return OK;
}

RESULT OpenGLRenderer::GetFrame( FramePtr& frame )
{
	return OK;
}

RESULT OpenGLRenderer::ReleaseFrame( FramePtr frame )
{
	return OK;
}

void OpenGLRenderer::paintEvent( QPaintEvent* )
{
	// Do nothing here, let render thread does rendering and prevent parent class to call makeCurrent()
}

void OpenGLRenderer::showEvent( QShowEvent*)
{
	if (!m_ReadyToRender) {
		doneCurrent();
		m_ReadyToRender = true;
	}
}

void OpenGLRenderer::OnResizeTimer()
{	
}

void OpenGLRenderer::resizeEvent( QResizeEvent* )
{
	// Do nothing, but prevent parent class to call makeCurrent()		

	if (width()!=m_BufferWidth || height()!=m_BufferHeight)
	{
		m_BufferWidth = width();
		m_BufferHeight = height();
		m_BufferSizeChanged = true;
	}
}
