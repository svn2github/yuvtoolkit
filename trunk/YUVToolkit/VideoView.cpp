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


unsigned int VideoView::m_IDCounter = 0;

VideoView::VideoView(QMainWindow* _mainWin, RendererWidget* _parent, ProcessThread* processThread, PlaybackControl* control) :
	parent(_parent), m_Type(PLUGIN_UNKNOWN), m_MainWindow(_mainWin), m_TransformActionListUpdated(false),
		m_Dock(NULL), m_PluginGUI(NULL), m_ProcessThread(processThread), m_Control(control)
{
	m_ViewID = m_IDCounter++;
	m_LastMousePoint.setX(-1);
	m_LastMousePoint.setY(-1);
	m_ScaleNum = 0;
	m_ScaleDen = 0;
	m_SrcLeft = 0;
	m_SrcTop = 0;
	m_SrcWidth = 0;
	m_SrcHeight = 0;
	m_VideoHeight = 0;
	m_VideoWidth = 0;
	m_Duration = 0;

	m_Transform = 0;
	m_Measure = 0; 
	m_SourceThread = 0;

	m_RenderFormat = FormatPtr(new FormatImpl);
	m_SourceFormat = FormatPtr(new FormatImpl); 
	// m_EmptyFrame = FramePtr(new FrameImpl);
	
	m_CloseAction = new QAction("Close", this);
	connect(m_CloseAction, SIGNAL(triggered()), this, SLOT(OnClose()));
}

void VideoView::Init( const char* path)
{
	m_Type = PLUGIN_SOURCE;
	m_SourceThread = new SourceThread(m_ViewID, m_Control, path);

	Source* source = m_SourceThread->GetSource();
	if (source && source->HasGUI())
	{
		m_Dock = new QDockWidget("Video Source Options", m_MainWindow);
		//m_Dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);		
		m_Dock->setAllowedAreas(NULL);

		m_PluginGUI = m_SourceThread->GetSource()->CreateGUI(m_Dock);
		m_Dock->setWidget(m_PluginGUI);

		m_Dock->setVisible(false);
		m_MainWindow->addDockWidget(Qt::BottomDockWidgetArea, m_Dock);
		m_Dock->setFloating(true);
		
		connect(m_Dock, SIGNAL(dockLocationChanged( Qt::DockWidgetArea)), m_MainWindow, SLOT(OnAutoResizeWindow()));
		connect(m_Dock, SIGNAL(topLevelChanged(bool)), this, SLOT(OnDockFloating(bool)));
		connect(m_Dock, SIGNAL(visibilityChanged(bool)), this, SLOT(OnDockFloating(bool)));

		source->GUINeeded.connect(this, &VideoView::OnSourceGUINeeded);
	}

	connect(m_SourceThread, SIGNAL(frameReady(FramePtr)), m_ProcessThread, SLOT(ReceiveFrame(FramePtr)));
}

void VideoView::Init( Transform* transform, VideoQueue* source, QString outputName )
{
	m_Type = PLUGIN_TRANSFORM;
	m_Transform = transform;
	// m_RefVideoQueue = source;
	m_OutputName = outputName;
}

void VideoView::Init( Measure* measure, VideoQueue* source, VideoQueue* source1 )
{
	m_Type = PLUGIN_TRANSFORM;
	m_Measure = measure;
	// m_RefVideoQueue = source;
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
	// TODO Clean up action list
//	delete m_VideoQueue;
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
	if (!m_VideoHeight || !m_VideoWidth)
	{
		return;
	}

	QRect rcClient(m_VideoViewRect);
	int dstLeft = 0;
	int dstTop = 0;
	int dstWidth = rcClient.width();
	int dstHeight = rcClient.height();

	m_SrcWidth = m_VideoWidth;
	m_SrcHeight = m_VideoHeight;
	
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
		dstWidth = m_VideoWidth*m_ScaleNum/m_ScaleDen;
		dstHeight = m_VideoHeight*m_ScaleNum/m_ScaleDen;

		if (dstWidth<=rcClient.width())
		{
			dstLeft = rcClient.left() + (rcClient.width()-dstWidth)/2;
			m_SrcLeft = 0;
		}else
		{
			dstLeft = rcClient.left();
			dstWidth = rcClient.width();

			m_SrcWidth = rcClient.width()*m_ScaleDen/m_ScaleNum;
			m_SrcLeft = min(max(m_SrcLeft, 0),m_VideoWidth-m_SrcWidth);
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
			m_SrcTop = min(max(m_SrcTop, 0),m_VideoHeight-m_SrcHeight);
		}
	}

	srcRect.setRect(m_SrcLeft, m_SrcTop, m_SrcWidth, m_SrcHeight);
	dstRect.setRect(dstLeft, dstTop, dstWidth, dstHeight);

	if (emitSignal)
	{
		double w = m_VideoWidth;
		double h = m_VideoHeight;

		emit ViewPortUpdated(this, (m_SrcLeft+m_SrcWidth/2)/w, (m_SrcTop+m_SrcHeight/2)/h);
	}
}

void VideoView::computeAR( int src_width, int src_height, int& win_width, int& win_height )
{
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

void VideoView::SetTitle( const char* title )
{
	m_Title = title;
}

bool VideoView::CheckResolutionDurationChanged()
{
	Source* source = VV_SOURCE(this);
	FramePtr frame = VV_LASTFRAME(this);

	UpdateTransformActionList();

	int width  = 0;
	int height = 0;
	int duration = 0;

	if (source)
	{
		SourceInfo info;
		source->GetInfo(info);
		width  = info.format->Width();
		height = info.format->Height();
		duration = info.duration;
	}else if (frame)
	{
		width = frame->Format()->Width();
		height = frame->Format()->Height();
	}
	
	if (m_VideoWidth != width || m_VideoHeight != height || m_Duration != duration)
	{
		m_VideoWidth = width;
		m_VideoHeight = height;
		m_Duration = duration;

		return true;
	}
	
	return false;
}

void VideoView::GetVideoSize( QSize& actual, QSize& display )
{
	actual.setWidth(m_VideoWidth);
	actual.setHeight(m_VideoHeight);

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
	int xCenter = (int)(x*m_VideoWidth);
	int yCenter = (int)(y*m_VideoHeight);

	m_SrcLeft += xCenter-(m_SrcLeft+m_SrcWidth/2);
	m_SrcTop  += yCenter-(m_SrcTop+m_SrcHeight/2);

	RepositionVideo();
}

void VideoView::UpdateTransformActionList()
{
	if (!m_TransformActionListUpdated)	
	{
		Source* source = GetSource();
		if (source)
		{
			SourceInfo info;
			source->GetInfo(info);

			const QList<PlugInInfo*>& lst = GetHostImpl()->GetTransformPluginList();
			
			for (int i=0; i<lst.size(); i++)
			{
				PlugInInfo* plugInInfo = lst[i];

				Transform* transform = plugInInfo->plugin->NewTransform(plugInInfo->string);
				
				QStringList outputNames;
				QStringList statNames;
				transform->GetSupportedModes(info.format, outputNames, statNames);

				for (int j=0; j<outputNames.size(); j++)
				{
					QString str;
					QTextStream(&str) << "Show " << outputNames.at(j);

					QAction* action = new QAction(str, this);
					action->setCheckable(true);
					action->setChecked(false);

					TransformActionData* data = new TransformActionData;
					data->transformPlugin = plugInInfo->plugin;
					data->transformName = plugInInfo->string;
					data->outputName = outputNames.at(j);
					action->setData(qVariantFromValue((void*)data));

					connect(action, SIGNAL(triggered()), this, SLOT(OnTransformTriggered()));
					m_TransformActionList.append(action);
				}
			}

			m_TransformActionListUpdated = true;
		}
	}
}

void VideoView::OnClose()
{
	emit Close(this);
}

void VideoView::OnTransformTriggered()
{
	QAction* action = qobject_cast<QAction*>(QObject::sender());

	TransformActionData* data = (TransformActionData*)action->data().value<void *>();
	
	emit TransformTriggered(action, this, data);
}

void VideoView::OnSourceGUINeeded()
{
	m_Dock->setVisible(true);
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
