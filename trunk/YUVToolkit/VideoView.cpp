#include "YT_Interface.h"
#include <QtGui>

#include "VideoView.h"
#include "RendererWidget.h"
#include "YT_InterfaceImpl.h"
#include "SourceThread.h"
#include "ProcessThread.h"


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


VideoView::VideoView(QMainWindow* _mainWin, unsigned int viewId, RendererWidget* _parent, ProcessThread* processThread, PlaybackControl* control) :
	m_Type(PLUGIN_UNKNOWN), m_SourceThread(NULL), m_ProcessThread(processThread),
	m_Control(control), m_Transform(0),m_Measure(0),
	m_MainWindow(_mainWin), m_Menu(NULL), m_CloseAction(0),
	m_ViewID(viewId),
	m_ScaleNum(1), m_ScaleDen(1), m_SrcLeft(0), m_SrcTop(0), m_SrcWidth(0), m_SrcHeight(0),
	m_Dock(NULL), m_PluginGUI(NULL), parent(NULL)
{
	m_LastMousePoint.setX(-1);
	m_LastMousePoint.setY(-1);

	m_CloseAction = new QAction("Close", this);

	connect(m_CloseAction, SIGNAL(triggered()), this, SLOT(close()));

	m_Menu = new QMenu(m_MainWindow);
}

void VideoView::Init( const char* path)
{
	m_Type = PLUGIN_SOURCE;
	m_SourceThread = new SourceThread(this, m_ViewID, m_Control, path);

	// connect(this, SIGNAL(ResolutionChanged()), m_SourceThread, SLOT(ResolutionDurationChanged()));
	connect(this, SIGNAL(ResolutionChanged()), m_MainWindow, SLOT(OnAutoResizeWindow()));
	connect(this, SIGNAL(SourceReset(unsigned int)), m_ProcessThread, SLOT(CleanFrameQueue(unsigned int)));
	connect(m_SourceThread, SIGNAL(frameReady(FramePtr)), m_ProcessThread, SLOT(ReceiveFrame(FramePtr)));
	connect(m_SourceThread, SIGNAL(sourceReset()), this, SLOT(OnSourceReset()));

	UpdateMenu();
	
	
	m_SourceThread->GetSource()->GetInfo(m_SourceInfo);
	emit ResolutionChanged();
}

void VideoView::Init( Transform* transform, VideoQueue* source, QString outputName )
{
	m_Type = PLUGIN_TRANSFORM;
	m_Transform = transform;
	// m_RefVideoQueue = source;
	m_OutputName = outputName;

	connect(this, SIGNAL(ResolutionChanged()), m_MainWindow, SLOT(OnAutoResizeWindow()));

	UpdateMenu();
}

void VideoView::Init(unsigned int source, unsigned int processed)
{
	m_Type = PLUGIN_MEASURE;

	connect(this, SIGNAL(ResolutionChanged()), m_MainWindow, SLOT(OnAutoResizeWindow()));

	UpdateMenu();
}

void VideoView::UnInit()
{
	m_LastFrame.clear();

	if (m_Dock)
	{
		m_MainWindow->removeDockWidget(m_Dock);
		delete m_Dock;
	}

	SourceThread* st = GetSourceThread();
	if (st)
	{

		st->Stop();

		SAFE_DELETE(st);
	}

// 	m_VideoQueue->ReleaseBuffers();
}

VideoView::~VideoView()
{
}


void VideoView::SetZoomLevel( int mode )
{
	switch (mode)
	{
	case 0:
		m_ScaleNum = 1;
		m_ScaleDen = 2;		
		break;
	case 1:
		m_ScaleNum = 1;
		m_ScaleDen = 1;
		break;
	case 2:
		m_ScaleNum = 2;
		m_ScaleDen = 1;
		break;
	case 3:
		m_ScaleNum = 4;
		m_ScaleDen = 1;
		break;
	case 4:
		m_ScaleNum = 0;
		m_ScaleDen = 0;		
		break;
	}

	// this->update();
	RepositionVideo();
}

void VideoView::RepositionVideo(bool emitSignal)
{
	if (!m_SourceInfo.format)
	{
		return;
	}

	QRect rcClient(m_VideoViewRect);
	int dstLeft = 0;
	int dstTop = 0;
	int dstWidth = rcClient.width();
	int dstHeight = rcClient.height();

	m_SrcWidth = width();
	m_SrcHeight = height();
	
	if (m_ScaleDen == 0 || m_ScaleNum == 0)
	{
		// fit to window
		computeAR(m_SrcWidth, m_SrcHeight, dstWidth, dstHeight );

		dstLeft = rcClient.left()+(rcClient.width()-dstWidth)/2;
		dstTop = rcClient.top()+(rcClient.height()-dstHeight)/2;

		m_SrcLeft = 0;
		m_SrcTop = 0;			
	}else
	{
		dstWidth = width()*m_ScaleNum/m_ScaleDen;
		dstHeight = height()*m_ScaleNum/m_ScaleDen;

		if (dstWidth<=rcClient.width())
		{
			dstLeft = rcClient.left() + (rcClient.width()-dstWidth)/2;
			m_SrcLeft = 0;
		}else
		{
			dstLeft = rcClient.left();
			dstWidth = rcClient.width();

			m_SrcWidth = rcClient.width()*m_ScaleDen/m_ScaleNum;
			m_SrcLeft = min(max(m_SrcLeft, 0),width()-m_SrcWidth);
		}

		if (dstHeight<=rcClient.height())
		{
			dstTop = rcClient.top() + (rcClient.height()-dstHeight)/2;
			m_SrcTop = 0;
		}else
		{
			dstTop = rcClient.top();
			dstHeight = rcClient.height();

			m_SrcHeight = rcClient.height()*m_ScaleDen/m_ScaleNum;
			m_SrcTop = min(max(m_SrcTop, 0),height()-m_SrcHeight);
		}
	}

	srcRect.setRect(m_SrcLeft, m_SrcTop, m_SrcWidth, m_SrcHeight);
	dstRect.setRect(dstLeft, dstTop, dstWidth, dstHeight);

	if (emitSignal)
	{
		double w = width();
		double h = height();

		emit ViewPortUpdated(this, (m_SrcLeft+m_SrcWidth/2)/w, (m_SrcTop+m_SrcHeight/2)/h);
	}
}

void VideoView::computeAR( int src_width, int src_height, int& win_width, int& win_height )
{
	if (!src_width || !src_height)
	{
		return;
	}
	int win_height2 = win_width*src_height/src_width;

	if (win_height2 > win_height)
	{
		win_width = win_height*src_width/src_height;
	}
	else
	{
		win_height = win_height2;
	}
}

void VideoView::SetGeometry( int x, int y, int width, int height )
{
	m_VideoViewRect.setRect(x, y, width, height);

	RepositionVideo();
}

void VideoView::setTitle( QString title )
{
	m_Title = title;

	QString str;
	QTextStream(&str) << "[" << QString("%1").arg(GetID()) << "] - " << m_Title;
	m_Menu->setTitle(str);
}

void VideoView::GetVideoSize( QSize& actual, QSize& display )
{
	actual.setWidth(width());
	actual.setHeight(height());

	if (m_ScaleDen == 0 || m_ScaleNum == 0)
	{
		display = actual;
	}else
	{
		display.setWidth(actual.width()*m_ScaleNum/m_ScaleDen);
		display.setHeight(actual.height()*m_ScaleNum/m_ScaleDen);
	}
}

void VideoView::OnMouseMoveEvent( const QPoint& pt)
{
	if (m_ScaleDen == 0 || m_ScaleNum == 0)
	{
		return;
	}

	if (m_LastMousePoint.x()>0)
	{
		m_SrcLeft -= (pt.x()-m_LastMousePoint.x())*m_ScaleDen/m_ScaleNum;
	}

	if (m_LastMousePoint.y()>0)
	{
		m_SrcTop -= (pt.y()-m_LastMousePoint.y())*m_ScaleDen/m_ScaleNum;
	}

	m_LastMousePoint = pt;

	RepositionVideo(true);
}

void VideoView::OnMousePressEvent(const QPoint& pt)
{
	m_LastMousePoint.setX(-1);
	m_LastMousePoint.setY(-1);
}

void VideoView::OnMouseReleaseEvent(const QPoint& pt)
{
	m_LastMousePoint.setX(-1);
	m_LastMousePoint.setY(-1);
}

void VideoView::UpdateViewPort( double x, double y )
{
	int xCenter = (int)(x*width());
	int yCenter = (int)(y*height());

	m_SrcLeft += xCenter-(m_SrcLeft+m_SrcWidth/2);
	m_SrcTop  += yCenter-(m_SrcTop+m_SrcHeight/2);

	RepositionVideo();
}

void VideoView::UpdateTransformActionList()
{
	
}

void VideoView::close()
{
	emit Close(this);
}

void VideoView::OnTransformTriggered()
{
	QAction* action = qobject_cast<QAction*>(QObject::sender());

	TransformActionData* data = (TransformActionData*)action->data().value<void *>();
	
	emit TransformTriggered(action, this, data);
}

void VideoView::ShowGui(Source* source, bool show)
{
	if (m_Dock == NULL)
	{
		if (source && source->HasGUI())
		{
			m_Dock = new QDockWidget("Video Source Options", m_MainWindow);
			//m_Dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);		
			m_Dock->setAllowedAreas(NULL);

			m_PluginGUI = source->CreateGUI(m_Dock);
			m_Dock->setWidget(m_PluginGUI);

			m_Dock->setVisible(false);
			m_MainWindow->addDockWidget(Qt::BottomDockWidgetArea, m_Dock);
			m_Dock->setFloating(true);

			connect(m_Dock, SIGNAL(dockLocationChanged( Qt::DockWidgetArea)), m_MainWindow, SLOT(OnAutoResizeWindow()));
			connect(m_Dock, SIGNAL(topLevelChanged(bool)), this, SLOT(OnDockFloating(bool)));
			connect(m_Dock, SIGNAL(visibilityChanged(bool)), this, SLOT(OnDockFloating(bool)));
		}
	}
	
	if (m_Dock && show)
	{
		m_Dock->setVisible(true);
	}
}


void VideoView::ResolutionDurationChanged()
{
	m_SourceInfo.Reset();

	if (m_SourceThread)
	{
		m_SourceThread->GetSource()->GetInfo(m_SourceInfo);

		m_SourceThread->ResetSource();
	}

	// Clean source queue in process thread
	// so source thread can read new frames
	emit SourceReset(m_ViewID);

	/*PlaybackControl::Status status = {0};
	m_Control->GetStatus(&status);
	m_Control->Seek(status.lastDisplayPTS);*/

	emit ResolutionChanged();
}


void VideoView::OnDockFloating(bool f)
{
	if (f)
	{
		m_Dock->resize(m_Dock->minimumSize());
	}
}

Source* VideoView::GetSource()
{
	if (m_SourceThread)
	{
		return m_SourceThread->GetSource();
	}else
	{
		return NULL;
	}
}

void VideoView::UpdateMenu()
{
	m_Menu->clear();
	if (m_Dock)
	{
		m_Menu->addAction(m_Dock->toggleViewAction());
	}

	m_Menu->addSeparator();
	m_Menu->addAction(GetCloseAction());
}

QMenu* VideoView::GetMenu()
{
	return m_Menu;
}

int VideoView::width()
{
	if (m_SourceInfo.format)
	{
		return m_SourceInfo.format->Width();
	}else
	{
		return 0;
	}
	
}

int VideoView::height()
{
	if (m_SourceInfo.format)
	{
		return m_SourceInfo.format->Height();
	}else
	{
		return 0;
	}
}

QString VideoView::title()
{
	return m_Title;
}

void VideoView::setTimeStamps( QList<unsigned int> lst)
{
	if (m_SourceThread)
	{
		m_SourceThread->GetSource()->SetTimeStamps(lst);
	}
}

void VideoView::SetLastFrame( FramePtr f )
{
	m_LastFrame = f;
	if (!m_SourceInfo.format)
	{
		m_SourceInfo.format = FormatPtr(new FormatImpl);
	}
	
	if (*(m_SourceInfo.format) != *(f->Format()))
	{
		*(m_SourceInfo.format) = *(f->Format());

		emit ResolutionChanged();
	}
}

void VideoView::OnSourceReset()
{
	PlaybackControl::Status status = {0};
	m_Control->GetStatus(&status);

	// Force Process Thread to load the last frame in the queue
	m_Control->Seek(status.lastDisplayPTS);
}
