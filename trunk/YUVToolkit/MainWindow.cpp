#include "MainWindow.h"
#include "SourceThread.h"

#include "YT_InterfaceImpl.h"
#include "RendererWidget.h"
#include "UI/ClickableSlider.h"
#include "VideoView.h"
#include "Layout.h"
#include "RenderThread.h"
#include "VideoViewList.h"

#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptValue>

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif


int MainWindow::windowCounter = 0;
MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : 
QMainWindow(parent, flags), m_IsPlaying(false), m_ActiveVideoView(0)
{
	if (windowCounter == 0)
	{
		GetHostImpl()->InitLogging();
	}
	windowCounter++;

	ui.setupUi(this);
	setMinimumWidth(320);
	setMinimumHeight(240);
	setCentralWidget(ui.rendererWidget);
	setAcceptDrops(TRUE);

	m_Slider = new QClickableSlider(ui.mainToolBar);
	m_Slider->setMinimumSize(32, 10);
	m_Slider->setFocusPolicy(Qt::NoFocus);
	m_Slider->setTickPosition(QSlider::TicksBelow);
	m_Slider->setTickInterval(1000);
	m_Slider->setSingleStep(1);
	
	m_VideoViewList = new VideoViewList(this, ui.rendererWidget);
	connect(m_VideoViewList, SIGNAL(ResolutionDurationChanged()), this, SLOT(OnAutoResizeWindow()));
	connect(m_VideoViewList, SIGNAL(VideoViewClosed(VideoView*)), this, SLOT(OnVideoViewClosed(VideoView*)));
	connect(m_VideoViewList, SIGNAL(VideoViewCreated(VideoView*)), this, SLOT(OnVideoViewCreated(VideoView*)));

	ui.mainToolBar->insertWidget(ui.action_Seek_Beginning, m_Slider);
	ui.mainToolBar->insertSeparator(ui.action_Seek_Beginning);

	autoResizeWindow();

	m_UpdateTimer = new QTimer(this);
	m_UpdateTimer->setInterval(300);
	connect(m_UpdateTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));
	m_UpdateTimer->start();

	connect(m_Slider, SIGNAL(valueChanged(int)), this, SLOT(seekVideoFromSlider()));
	connect(this, SIGNAL(activeVideoViewChanged(VideoView*)), this, SLOT(OnActiveVideoViewChanged(VideoView*)));
	connect(ui.rendererWidget, SIGNAL(repositioned()), m_VideoViewList, SLOT(OnUpdateRenderWidgetPosition()));
	// ui.action_Step_Forward->setEnabled(true);
	// connect(ui.action_Step_Forward, SIGNAL(clicked()), m_Slider, SLOT(setSingleStep(1)));

	EnableButtons(false);	

	m_TimeLabel1 = new QLabel(this);
	ui.statusBar->addPermanentWidget(m_TimeLabel1);
	m_TimeLabel2 = new QLabel(this);
	ui.statusBar->addPermanentWidget(m_TimeLabel2);
	m_ZoomLabel = new QLabel(this);
	ui.statusBar->addPermanentWidget(m_ZoomLabel);
	m_RenderSpeedLabel = new QLabel(this);
	ui.statusBar->addPermanentWidget(m_RenderSpeedLabel);

	QSettings settings;
	m_RenderType = settings.value("main/renderer", "D3D").toString();

	QActionGroup* actionGroup;
	actionGroup = new QActionGroup(this);
	const QList<PlugInInfo*>& renderList = GetHostImpl()->GetRenderPluginList();
	for (int i=0; i<renderList.count(); i++)
	{
		PlugInInfo* plugin = renderList.at(i);
		
		QAction* action = new QAction(this);
		action->setObjectName(plugin->string);
		action->setText(plugin->string);
		action->setCheckable(true);

		m_RendererList.append(action);

		ui.menu_Renderer->addAction(action);
		actionGroup->addAction(action);

		connect(action, SIGNAL(triggered()), this, SLOT(OnRendererSelected()));

		if (plugin->string == m_RenderType)
		{
			action->setChecked(true);
		}		
	}
	
	actionGroup = new QActionGroup(this);
	actionGroup->addAction(ui.action_Zoom_Fit);	
	actionGroup->addAction(ui.action_Zoom_50);	
	actionGroup->addAction(ui.action_Zoom_100);	
	actionGroup->addAction(ui.action_Zoom_200);	
	actionGroup->addAction(ui.action_Zoom_400);	

	m_ZoomMode = settings.value("main/zoom", 1).toInt();
	m_ZoomMode = qMin(qMax(m_ZoomMode, 0), 4);
	switch (m_ZoomMode)
	{
	case 0:
		ui.action_Zoom_50->setChecked(true);
		break;
	case 1:
		ui.action_Zoom_100->setChecked(true);
		break;
	case 2:
		ui.action_Zoom_200->setChecked(true);
		break;
	case 3:
		ui.action_Zoom_400->setChecked(true);
		break;
	case 4:
		ui.action_Zoom_Fit->setChecked(true);
		break;
	}

	SetZoomMode(m_ZoomMode);

	ui.menu_Tools->addSeparator();
	const QList<QDockWidget*> dockList = m_VideoViewList->GetDockWidgetList();
	for (int i=0; i<dockList.size(); ++i) 
	{
		QDockWidget* dock = dockList.at(i);

		ui.menu_Tools->addAction(dock->toggleViewAction());
	}
}

MainWindow::~MainWindow()
{
	delete m_VideoViewList;
}

void MainWindow::closeEvent( QCloseEvent *event )
{
	while (m_VideoViewList->size())
	{
		m_VideoViewList->CloseVideoView(m_VideoViewList->last());
	}

	windowCounter--;
	if (windowCounter == 0)
	{
		GetHostImpl()->UnInitLogging();
	}
}

void MainWindow::on_action_Play_Pause_triggered(bool)
{
	play(!m_VideoViewList->IsPlaying());
}

void MainWindow::dragEnterEvent( QDragEnterEvent *event )
{
	// accept just text/uri-list mime format
	if (event->mimeData()->hasUrls())
	{     
		event->acceptProposedAction();
	}
}

void MainWindow::dropEvent( QDropEvent *event )
{
	QList<QUrl> urlList;
	QString fName;
	QFileInfo info;
	QStringList fileList;

	// if (event->mimeData()->hasUrls())
	{
		urlList = event->mimeData()->urls(); // returns list of QUrls

		// if just text was dropped, urlList is empty (size == 0)
		for ( int i=0; i<urlList.size(); i++) // if at least one QUrl is present in list
		{
			fName = urlList[i].toLocalFile(); // convert first QUrl to local path
			info.setFile( fName ); // information about file
			if ( info.isFile() )
			{
				fileList.append(fName);
			}
		}
	}

	event->acceptProposedAction();

	openFiles(fileList);
}


void MainWindow::openFiles( const QStringList& fileList )
{
	QStringList fileList2;
	for ( int i=0; i<fileList.size(); i++) // if at least one QUrl is present in list
	{
		const QString& fName = fileList[i];
		if (fName.size()<=4)
		{
			continue;
		}
		if (fName.right(4).compare(".yts", Qt::CaseInsensitive) == 0)
		{
			openScript(fName, false);
		}else
		{
			fileList2.append(fName);
		}
	}

	if (fileList2.size() == 0)
	{
		return;
	}

	bool playNow = true;
	if (m_VideoViewList->size()>0)
	{
		playNow = m_VideoViewList->IsPlaying();
		m_VideoViewList->Seek(INVALID_PTS, false);
	}
	
	for ( int i=0; i<fileList2.size(); i++) // if at least one QUrl is present in list
	{
		const QString& fName = fileList2[i];
		openFileInternal(fName);
	}

	m_VideoViewList->Seek(INVALID_PTS, playNow);
}


void MainWindow::on_action_Open_triggered()
{
	QString openFilesPath;
	QFileDialog::Options options;
	QString selectedFilter;
	QStringList files = QFileDialog::getOpenFileNames(
		this, ("QFileDialog::getOpenFileNames()"),
		openFilesPath,
		("All Files (*);;YUVToolkit Script (YTS) Files (*.yts)"),
		&selectedFilter,
		options);

	openFiles(files);
}


void MainWindow::on_action_Run_Script_triggered()
{
	QString openFilesPath;
	QFileDialog::Options options;
	QString selectedFilter;
	QStringList fileList = QFileDialog::getOpenFileNames(
		this, ("QFileDialog::getOpenFileNames()"),
		openFilesPath,
		("YUVToolkit Script (YTS) Files (*.yts);;All Files (*)"),
		&selectedFilter,
		options);

	for ( int i=0; i<fileList.size(); i++)
	{
		const QString& fName = fileList[i];

		openScript(fName, false);
	}
}

void MainWindow::on_action_Debug_Script_triggered()
{
	QString openFilesPath;
	QFileDialog::Options options;
	QString selectedFilter;
	QStringList fileList = QFileDialog::getOpenFileNames(
		this, ("QFileDialog::getOpenFileNames()"),
		openFilesPath,
		("YUVToolkit Script (YTS) Files (*.yts);;All Files (*)"),
		&selectedFilter,
		options);

	for ( int i=0; i<fileList.size(); i++)
	{
		const QString& fName = fileList[i];

		openScript(fName, true);
	}
}


void MainWindow::on_action_Close_triggered()
{
	m_UpdateTimer->stop();

	VideoView* vv = NULL;
	if (m_ActiveVideoView)
	{
		vv = m_ActiveVideoView;		
	}else if (m_VideoViewList->size()>0)
	{
		vv = m_VideoViewList->last();
	}

	if (vv)
	{
		m_VideoViewList->CloseVideoView(vv);
		
		m_UpdateTimer->start();
	}else
	{
		this->close();
	}
}

void MainWindow::on_action_Exit_triggered()
{
	this->close();
}

void MainWindow::on_action_Zoom_50_triggered()
{
	SetZoomMode(0);
}

void MainWindow::on_action_Zoom_100_triggered()
{
	SetZoomMode(1);
}

void MainWindow::on_action_Zoom_200_triggered()
{
	SetZoomMode(2);
}

void MainWindow::on_action_Zoom_400_triggered()
{
	SetZoomMode(3);
}

void MainWindow::on_action_Zoom_Fit_triggered()
{
	SetZoomMode(4);
}

void MainWindow::on_action_Zoom_Switch_triggered()
{
	SetZoomMode((m_ZoomMode+1)%5);
}

void MainWindow::SetZoomMode( int mode )
{
	if (mode != m_ZoomMode)
	{
		m_ZoomMode = mode;
	
		for (int i=0; i<m_VideoViewList->size(); ++i) 
		{
			VideoView* vv = m_VideoViewList->at(i);
			vv->SetZoomLevel(m_ZoomMode);
		}

		autoResizeWindow();

		QSettings settings;
		settings.setValue("main/zoom", m_ZoomMode);
	}	

	switch(mode)
	{
	case 0:
		m_ZoomLabel->setText("50 %");
		break;
	case 1:
		m_ZoomLabel->setText("100 %");
		break;
	case 2:
		m_ZoomLabel->setText("200 %");
		break;
	case 3:
		m_ZoomLabel->setText("400 %");
		break;
	case 4:
		m_ZoomLabel->setText("Auto Fit");
		break;
	}
}



void MainWindow::openFile( QString strPath)
{
	bool playNow = true;
	if (m_VideoViewList->size()>0)
	{
		playNow = m_VideoViewList->IsPlaying();
		m_VideoViewList->Seek(INVALID_PTS, false);
	}

	openFileInternal(strPath);
	
	m_VideoViewList->Seek(INVALID_PTS, playNow);
}

void MainWindow::openFileInternal( QString strPath)
{
	QStringList fileList;
	for (int i=0; i<m_VideoViewList->size(); ++i) 
	{
		SourceThread* st = m_VideoViewList->at(i)->GetSourceThread();
		if (st)
		{
			if (QString::compare(st->GetSourcePath(), strPath, Qt::CaseInsensitive) == 0)
			{
				return;
			}
		}
	}

	QFile file(strPath);
	if (!file.exists())
	{		
		QMessageBox::warning(this, "Error", "File does not exist.");
		return;
	}

	VideoView* vv = m_VideoViewList->NewVideoView(strPath.toAscii());
	vv->Init(strPath.toAscii(), m_VideoViewList->GetCurrentPTS());

	m_VideoViewList->UpdateDuration();

	EnableButtons(true);
}



void MainWindow::play( bool play )
{
	m_VideoViewList->Seek(INVALID_PTS, play);
}


void MainWindow::infoMsg( QString title, QString msg )
{
	QMessageBox::information(this, title, msg, QMessageBox::Ok);
}

void MainWindow::openScript( QString strPath, bool debug )
{
	 QScriptEngineDebugger *debugger = NULL;
	 QMainWindow* debuggerWindow = NULL;
	 QScriptEngine engine(this);
	 engine.setProcessEventsInterval(100);

	 QFile file(strPath);
	 if (file.size()>1000000)
	 {
		 QMessageBox::warning(this, "Error", "File is too big. Cannot open it as script.");
		 return;
	 }
	 file.open(QIODevice::ReadOnly);
	 QString contents = file.readAll();
	 file.close();

	 if (debug) 
	 {
		 if (!debugger) 
		 {
			 debugger = new QScriptEngineDebugger(this);
			 debuggerWindow = debugger->standardWindow();
			 debuggerWindow->setWindowModality(Qt::ApplicationModal);
			 // debuggerWindow->resize(1280, 704);
		 }
		 debugger->attachTo(&engine);
		 debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
	 }

	 QScriptValue scriptMainWindow = engine.newQObject(this);
	 engine.globalObject().setProperty("yt", scriptMainWindow);

	 QString oldPath = QDir::currentPath();
	 QFileInfo fileInfo(strPath);
	 QDir::setCurrent(fileInfo.path());
	 QScriptValue ret = engine.evaluate(contents, strPath);
	 QDir::setCurrent(oldPath);

	 if (debuggerWindow)
	 {
		 debuggerWindow->hide();
	 }

	 if (debugger)
	 {
		 debugger->detach();
		 SAFE_DELETE(debugger);
	 }

	 if (ret.isError())
	 {
		 QMessageBox::warning(this, tr("Context 2D"), tr("Line %0: %1")
			 .arg(ret.property("lineNumber").toInt32())
			 .arg(ret.toString()));
	 }
}

void MainWindow::autoResizeWindow()
{
	QSize display;
	QSize window = ui.rendererWidget->size();

	if (isMaximized() || isMinimized())
	{
		m_VideoViewList->OnUpdateRenderWidgetPosition();
		return;
	}
	
	if (ui.rendererWidget->layout->GetVideoCount()>0)
	{
		QSize newSize;
		ui.rendererWidget->layout->GetDisplaySize(display);

		newSize.setWidth(rect().width()+(display.width()-window.width()));
		newSize.setHeight(rect().height()+(display.height()-window.height()));

#if defined(Q_WS_X11)
		QRect screen = QApplication::desktop()->availableGeometry(x11Info().screen());
#else // all others
		QRect screen = QApplication::desktop()->availableGeometry(pos());
#endif
		screen.setRight(screen.right()-16);
		screen.setBottom(screen.bottom()-48);

		newSize.setWidth(qMin(newSize.width(), screen.width()));
		newSize.setHeight(qMin(newSize.height(), screen.height()));

		newSize.setWidth(qMax(newSize.width(), minimumWidth()));
		newSize.setHeight(qMax(newSize.height(), minimumHeight()));

		if (m_ZoomMode == 4)
		{
			// In auto Zoom mode, only allow increasing in size
			newSize.setWidth(qMax(newSize.width(), rect().width()));
			newSize.setHeight(qMax(newSize.height(), rect().height()));
		}

		resize(newSize);

		if (pos().x()+newSize.width()>screen.right() || 
			pos().y()+newSize.height()>screen.bottom())
		{
			this->move(qMin(pos().x(), screen.right()-newSize.width()),
				qMin(pos().y(), screen.bottom()-newSize.height()));
		}
		// adjustSize();
	}else
	{
		resize(480, 420);
	}

	m_VideoViewList->OnUpdateRenderWidgetPosition();
}

void MainWindow::on_action_File_Association()
{
/*
	LPAPPASSOCREGUI p_appassoc;
    CoInitialize( 0 );

    if( S_OK == CoCreateInstance( &clsid_IApplication2,
                NULL, CLSCTX_INPROC_SERVER,
                &IID_IApplicationAssociationRegistrationUI,
                (void **)&p_appassoc) )
    {
        if(S_OK == p_appassoc->vt->LaunchAdvancedAssociationUI(p_appassoc, L"YUVToolkit" ) )
        {
            CoUninitialize();
            return;
        }
    }

    CoUninitialize();

*/}

void MainWindow::on_action_New_Window_triggered()
{
	QMainWindow* w = GetHostImpl()->NewMainWindow(0, NULL);
	w->show();
}

void MainWindow::on_action_About_triggered()
{
	QPixmap logo(QString::fromUtf8(":/RawVideoToolkit/Resources/YuvToolkit.png"), "PNG");

	QMessageBox about(this);
	about.setWindowTitle("About");

	QString str;
	QTextStream(&str) << "<H1>YUV Toolkit</H1>"
		<< "<B> Version " <<
#include "../Setup/VERSION_1"
		<< "." << 
#include "../Setup/VERSION_2"
		<< "." << 
#include "../Setup/VERSION_3"
		<< " Build " <<
#include "../Setup/VERSION_4"
		<< "</B><BR><BR>"
		<< "<I>Compiled " << __DATE__ << " "  << __TIME__ << "</I>"
		<< "<BR><BR>Copyright David Zhao (C) 2009-2011"
		<< "<BR>All Rights Reserved";
	about.setText(str);

	about.setIconPixmap(logo.scaledToWidth(128));
	about.exec();
}

void MainWindow::OnUpdateSlider(unsigned int duration, float fps, unsigned int pts)
{
	int ticks = qCeil(duration * fps / 1000)-1;
	if (m_Slider->maximum() != ticks)
	{
		bool old = m_Slider->blockSignals(true);
		m_Slider->setRange(0, ticks);
		m_Slider->setSingleStep(1);
		m_Slider->blockSignals(old);
	}

	int pos = qRound(pts * fps / 1000);
	if (m_Slider->sliderPosition() != pos)
	{
		bool old = m_Slider->blockSignals(true);
		m_Slider->setSliderPosition(pos);
		m_Slider->blockSignals(old);
	}
}

void MainWindow::seekVideoFromSlider()
{
	VideoView* longest = m_VideoViewList->longest();
	if (!longest)
	{
		return;
	}

	YT_Source_Info info;
	longest->GetSource()->GetInfo(info);

	int idx_new = m_Slider->value();

	for (int i=idx_new-1; i<idx_new+1; i++)
	{
		unsigned int pts = qFloor(idx_new * 1000 / info.fps);
		if (qRound(pts * info.fps / 1000) == idx_new)
		{
			m_VideoViewList->Seek(pts, false);

			return;
		}
	}
	
	
}

void MainWindow::on_action_Step_Forward_triggered()
{
	stepVideo(1);
}

void MainWindow::on_action_Step_Back_triggered()
{
	stepVideo(-1);
}

void MainWindow::on_action_Step_Forward_Fast_triggered()
{
	stepVideo(10);
}

void MainWindow::on_action_Step_Back_Fast_triggered()
{
	stepVideo(-10);
}

void MainWindow::on_action_Seek_Beginning_triggered()
{
	m_VideoViewList->Seek(0, false);
}

void MainWindow::on_action_Seek_End_triggered()
{
	VideoView* longest = m_VideoViewList->longest();
	if (longest)
	{
		YT_Source* source = longest->GetSource();
		
		YT_Source_Info info;
		source->GetInfo(info);

		m_VideoViewList->Seek(info.duration, false);
	}
}

void MainWindow::OnTimer()
{
	if (m_VideoViewList->size()>0)
	{
		m_VideoViewList->CheckResolutionChanged();
		m_VideoViewList->CheckRenderReset();
		m_VideoViewList->CheckSeeking();

		if (m_VideoViewList->IsPlaying())
		{
			m_VideoViewList->CheckLoopFromStart();
		}

		if (m_VideoViewList->size()>1)
		{
			m_VideoViewList->UpdateMeasureWindows();
		}
	}
	UpdateActiveVideoView();

	VideoView* active = m_ActiveVideoView;
	VideoView* longest = m_VideoViewList->longest();
	if (longest)
	{
		YT_Source* source = VV_SOURCE(longest);
		YT_Frame_Ptr frame = VV_LASTFRAME(longest);
		
		if (source && frame)
		{
			YT_Source_Info info;
			source->GetInfo(info);

			OnUpdateSlider(info.duration, info.fps, frame->PTS());
		}
	}

	if (active)
	{
		UpdateStatusMessage(active->GetTitle());
	}else
	{
		active = longest;
	}

	QString str;
	if (active)
	{
		YT_Source* source = VV_SOURCE(active);
		YT_Frame_Ptr frame = VV_LASTFRAME(active);

		if (source && frame)
		{
			YT_Source_Info info;
			source->GetInfo(info);

			str.clear();
			QTextStream(&str) << "" << frame->FrameNumber() << " / " << info.num_frames << "";
			m_TimeLabel1->setText(str);

			str.clear();
			QTextStream(&str) << frame->PTS() << " / " << info.duration << " ms";
			m_TimeLabel2->setText(str);			
		}

		if (m_VideoViewList->IsPlaying())
		{
			str.clear();
			float renderSpeed = m_VideoViewList->GetRenderThread()->GetSpeedRatio();
			QTextStream(&str) << QString("%1").arg(renderSpeed, 0, 'f', 2) << " x";
			m_RenderSpeedLabel->setText(str);
		}else
		{
			unsigned int pts = m_VideoViewList->GetSeekingPTS();
			if (pts != INVALID_PTS)
			{
				str.clear();
				QTextStream(&str) << "Seeking " << pts << " ms";
				m_RenderSpeedLabel->setText(str);
			}else
			{
				m_RenderSpeedLabel->setText("Paused");
			}
		}
	}else
	{
		m_TimeLabel1->setText("   ");
		m_TimeLabel2->setText("   ");
		m_RenderSpeedLabel->setText("   ");
	}

	if (m_VideoViewList->IsPlaying())
	{
		if (!m_IsPlaying)
		{
			QIcon icon;
			icon.addFile(QString::fromUtf8(":/RawVideoToolkit/Resources/media-playback-pause.png"), QSize(), QIcon::Normal, QIcon::Off);
			ui.action_Play_Pause->setIcon(icon);
			m_IsPlaying = true;
		}
	}else
	{
		if (m_IsPlaying)
		{
			QIcon icon;
			icon.addFile(QString::fromUtf8(":/RawVideoToolkit/Resources/media-playback-start.png"), QSize(), QIcon::Normal, QIcon::Off);
			ui.action_Play_Pause->setIcon(icon);
			m_IsPlaying = false;
		}
	}
	
	ui.action_Enable_Logging->setChecked(GetHostImpl()->IsLoggingEnabled());
}

void MainWindow::stepVideo( int step )
{
	YT_Source* source = 0;
	VideoView* longest = m_VideoViewList->longest();
	YT_Frame_Ptr lastFrame = 0;
	if (longest)
	{
		source = longest->GetSource();
		lastFrame = longest->GetVideoQueue()->GetLastRenderFrame()->source;
	}

	if (!source || !lastFrame)
	{
		return;
	}

	YT_Source_Info info;
	source->GetInfo(info);

	int frame_num = MIN(MAX(((int)lastFrame->FrameNumber()) + step, 0), info.num_frames-1);
	unsigned int pts = source->IndexToPTS(frame_num);

	m_VideoViewList->Seek(pts, false);

	bool old = m_Slider->blockSignals(true);
	m_Slider->setSliderPosition(frame_num);
	m_Slider->blockSignals(old);
}

void MainWindow::EnableButtons( bool enable )
{
	ui.action_Play_Pause->setEnabled(enable);
	ui.action_Step_Back->setEnabled(enable);
	ui.action_Step_Back_Fast->setEnabled(enable);
	ui.action_Step_Forward->setEnabled(enable);
	ui.action_Step_Forward_Fast->setEnabled(enable);
	ui.action_Seek_Beginning->setEnabled(enable);
	ui.action_Seek_End->setEnabled(enable);
	m_Slider->setEnabled(enable);

	if (!enable)
	{
		m_Slider->setSliderPosition(0);
	}
}


void MainWindow::OnAutoResizeWindow()
{
	autoResizeWindow();
}

void MainWindow::on_action_Enable_Logging_triggered()
{
	GetHostImpl()->EnableLogging(ui.action_Enable_Logging->isChecked());
}

void MainWindow::on_actionShowLogging_triggered()
{
	GetHostImpl()->OpenLoggingDirectory();
}

void MainWindow::OnRendererSelected()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
	{
		QStringList fileList;
		for (int j=0; j<m_VideoViewList->size(); ++j) 
		{
			SourceThread* graph = m_VideoViewList->at(j)->GetSourceThread();
			if (graph)
			{
				fileList.append(graph->GetSourcePath());
			}
		}

		while (m_VideoViewList->size()>0) 
		{
			VideoView* vv = m_VideoViewList->first();
			m_VideoViewList->CloseVideoView(vv);
		}

		m_RenderType = action->objectName();

		QSettings settings;
		settings.setValue("main/renderer", m_RenderType);

		openFiles(fileList);

		UpdateActiveVideoView();
	}
}

void MainWindow::UpdateActiveVideoView()
{
	VideoView* view = ui.rendererWidget->layout->FindVideoAtMoisePosition();

	if (view != m_ActiveVideoView)
	{
		m_ActiveVideoView = view;
		emit activeVideoViewChanged(view);
	}
}

void MainWindow::OnActiveVideoViewChanged( VideoView* view)
{
	if (view)
	{
		UpdateStatusMessage(view->GetTitle());
	}else
	{
		ui.statusBar->clearMessage();
	}
}


void MainWindow::UpdateStatusMessage( const QString& msg )
{
	int width = ui.statusBar->width();
	width -= m_TimeLabel1->width();
	width -= m_TimeLabel2->width();
	width -= m_ZoomLabel->width();
	width -= m_RenderSpeedLabel->width();
	width -= ui.statusBar->height()*3;
	if (width>0)
	{
		const QString elidedText = fontMetrics().elidedText(msg, Qt::ElideMiddle, width);
		ui.statusBar->showMessage(elidedText);
	}
}


void MainWindow::contextMenuEvent( QContextMenuEvent *event )
{
	QMenu menu(this);

	UpdateActiveVideoView();

	if (m_VideoViewList->size()==0 || m_ActiveVideoView == NULL)
	{
		menu.addAction(ui.action_Open);
		menu.addAction(ui.action_New_Window);
		menu.addSeparator();
		menu.addAction(ui.action_About);
	}else
	{
		QMenu* planeMenu = menu.addMenu(tr("&Zoom"));
		planeMenu->addAction(ui.action_Zoom_50);
		planeMenu->addAction(ui.action_Zoom_100);
		planeMenu->addAction(ui.action_Zoom_200);
		planeMenu->addAction(ui.action_Zoom_400);
		planeMenu->addAction(ui.action_Zoom_Fit);

		QDockWidget* dock = m_ActiveVideoView->GetDocketWidget();
		if (dock)
		{
			menu.addSeparator();
			menu.addAction(dock->toggleViewAction());
		}

		const QList<QAction*>& actionList = m_ActiveVideoView->GetTransformActions();
		if (actionList.size()>0)
		{
			menu.addSeparator();
			for (int i=0; i<actionList.size(); ++i)
			{
				menu.addAction(actionList[i]);
			}
		}

		menu.addSeparator();
		menu.addAction(m_ActiveVideoView->GetCloseAction());
	}
	
	menu.exec(event->globalPos());
}

void MainWindow::OnVideoViewClosed(VideoView* vv)
{
	m_ActiveVideoView = NULL;
	emit activeVideoViewChanged(NULL);

	if (vv->GetType() == YT_PLUGIN_TRANSFORM)
	{
		VideoQueue* vq = vv->GetRefVideoQueue();
		for (int i=0; i<m_VideoViewList->size(); i++)
		{
			VideoView* vvRef = m_VideoViewList->at(i);
			if (vq == vvRef->GetVideoQueue())
			{
				const QList<QAction*>& actions = vvRef->GetTransformActions();
				for (int j=0; j<actions.size(); j++)
				{
					TransformActionData* data = (TransformActionData*)actions[j]->data().value<void *>();
					if (data->outputName == vv->GetTitle())
					{
						actions[j]->setChecked(false);
					}
				}
			}
		}
	}

	if (m_VideoViewList->size() == 0)
	{
		EnableButtons(false);
	}

	autoResizeWindow();	
}

void MainWindow::OnVideoViewCreated( VideoView* vv)
{
	vv->SetZoomLevel(m_ZoomMode);
}

void MainWindow::on_action_Quality_Measures_triggered()
{
	
}

