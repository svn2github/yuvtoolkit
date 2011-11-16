#include "MainWindow.h"
#include "SourceThread.h"

#include "YT_InterfaceImpl.h"
#include "RendererWidget.h"
#include "UI/ClickableSlider.h"
#include "VideoView.h"
#include "Layout.h"
#include "RenderThread.h"
#include "VideoViewList.h"
#include "MeasureWindow.h"

#include <QScriptEngine>
#include <QScriptEngineDebugger>
#include <QScriptValue>

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MINIMUM_WIDTH 520
#define MINIMUM_HEIGHT 400

#define SHOW_NEW_FEATURES 0
#define SLIDER_STEP_MS  100

int MainWindow::windowCounter = 0;
MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : 
QMainWindow(parent, flags), m_IsPlaying(false), m_ActiveVideoView(0)
{
	windowCounter++;

	ui.setupUi(this);
	setMinimumWidth(MINIMUM_WIDTH);
	setMinimumHeight(MINIMUM_HEIGHT);
	setCentralWidget(ui.rendererWidget);
	setAcceptDrops(TRUE);

	m_Slider = new QClickableSlider(ui.playbackToolBar);
	m_Slider->setMinimumSize(32, 10);
	m_Slider->setFocusPolicy(Qt::NoFocus);
	m_Slider->setTickPosition(QSlider::TicksBelow);
	m_Slider->setTickInterval(1000/SLIDER_STEP_MS);
	m_Slider->setSingleStep(1);
	
	m_VideoViewList = new VideoViewList(this, ui.rendererWidget);
	connect(m_VideoViewList, SIGNAL(ResolutionDurationChanged()), this, SLOT(OnAutoResizeWindow()));
	connect(m_VideoViewList, SIGNAL(VideoViewClosed(VideoView*)), this, SLOT(OnVideoViewClosed(VideoView*)));
	connect(m_VideoViewList, SIGNAL(VideoViewCreated(VideoView*)), this, SLOT(OnVideoViewCreated(VideoView*)));
	connect(m_VideoViewList, SIGNAL(VideoViewListChanged()), this, SLOT(OnVideoViewListChanged()));
	connect(m_VideoViewList, SIGNAL(VideoViewSourceListChanged()), this, SLOT(OnVideoViewSourceListChanged()));
	
	ui.playbackToolBar->insertWidget(ui.action_Seek_Beginning, m_Slider);
	ui.playbackToolBar->insertSeparator(ui.action_Seek_Beginning);

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
	
	m_ZoomGroup = new QActionGroup(this);
	m_ZoomGroup->addAction(ui.action_Zoom_50);	
	m_ZoomGroup->addAction(ui.action_Zoom_100);	
	m_ZoomGroup->addAction(ui.action_Zoom_200);	
	m_ZoomGroup->addAction(ui.action_Zoom_400);	
	m_ZoomGroup->addAction(ui.action_Zoom_Fit);	

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

	ui.mainToolBar->addSeparator();
	QToolButton* zoomButton = new QToolButton( ui.mainToolBar );	
	ui.mainToolBar->addWidget(zoomButton);
	zoomButton->setDefaultAction(ui.action_Zoom_Switch);
	// zoomButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	QMenu *zoomMenu = new QMenu( zoomButton );
	zoomButton->setMenu( zoomMenu );
	zoomButton->setPopupMode( QToolButton::MenuButtonPopup );
	zoomMenu->addAction( ui.action_Zoom_50 );
	zoomMenu->addAction( ui.action_Zoom_100 );
	zoomMenu->addAction( ui.action_Zoom_200 );
	zoomMenu->addAction( ui.action_Zoom_400 );
	zoomMenu->addSeparator();
	zoomMenu->addAction( ui.action_Zoom_Fit );

#if SHOW_NEW_FEATURES
	ui.mainToolBar->addAction(ui.action_Overlay);
	ui.action_Overlay->setEnabled(false);
#endif
	ui.mainToolBar->addSeparator();

	m_ColorGroup = new QActionGroup(this);
	m_ColorGroup->addAction(ui.action_Color);
	m_ColorGroup->addAction(ui.action_Y);	
	m_ColorGroup->addAction(ui.action_U);
	m_ColorGroup->addAction(ui.action_V);
	ui.action_Color->setChecked(true);
	connect(m_ColorGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnColorActionTriggered(QAction*)));

	ui.mainToolBar->addAction(ui.action_Color);
	ui.mainToolBar->addAction(ui.action_Y);
	ui.mainToolBar->addAction(ui.action_U);
	ui.mainToolBar->addAction(ui.action_V);
	ui.mainToolBar->addSeparator();

	QIcon iconPlane;
	iconPlane.addFile(QString::fromUtf8(":/RawVideoToolkit/Resources/plane.png"), QSize(), QIcon::Normal, QIcon::Off);
	m_ActionsButton = new QToolButton( ui.mainToolBar );
	ui.mainToolBar->addWidget(m_ActionsButton);	
	m_ActionsButton->setIcon(iconPlane);
	m_ActionsButton->setText(QApplication::translate("MainWindow", "Actions", 0, QApplication::UnicodeUTF8));
	m_ActionsButton->setShortcut(tr("ALT+A"));
	m_ActionsButton->setToolTip(QApplication::translate("MainWindow", "Show actions for opened videos (Alt+A)", 0, QApplication::UnicodeUTF8));
	m_ActionsButton->setStatusTip(QApplication::translate("MainWindow", "Show actions for opened videos (Alt+A)", 0, QApplication::UnicodeUTF8));
	m_ActionsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	m_ActionsButton->setPopupMode( QToolButton::InstantPopup);

	m_CompareButton = new QToolButton( ui.mainToolBar );
	ui.mainToolBar->addWidget(m_CompareButton);	
	m_CompareButton->setDefaultAction(ui.action_Compare);
	m_CompareButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

#if SHOW_NEW_FEATURES
	QIcon iconGraph;
	iconGraph.addFile(QString::fromUtf8(":/RawVideoToolkit/Resources/utilities-system-monitor.png"), QSize(), QIcon::Normal, QIcon::Off);
	QToolButton* graphButton = new QToolButton( ui.mainToolBar );
	ui.mainToolBar->addWidget(graphButton);	
	graphButton->setIcon(iconGraph);
	graphButton->setText(QApplication::translate("MainWindow", "&Graph", 0, QApplication::UnicodeUTF8));
	graphButton->setToolTip(QApplication::translate("MainWindow", "Compare videos and show graph", 0, QApplication::UnicodeUTF8));
	graphButton->setStatusTip(QApplication::translate("MainWindow", "Compare videos and show graph with results", 0, QApplication::UnicodeUTF8));
	graphButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	graphButton->setEnabled(false);
#endif
	
	QString str("Compare");
	m_MeasureDockWidget= new QDockWidget(str, this );
	m_MeasureDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	m_MeasureDockWidget->setVisible(false);

	m_MeasureWindow = new MeasureWindow(m_VideoViewList, m_MeasureDockWidget);
	m_MeasureDockWidget->setWidget(m_MeasureWindow);
	addDockWidget(Qt::LeftDockWidgetArea, m_MeasureDockWidget);
	connect(m_VideoViewList, SIGNAL(VideoViewSourceListChanged()), m_MeasureWindow, SLOT(OnVideoViewSourceListChanged()));
	connect(ui.action_Distortion_Map, SIGNAL(toggled(bool)), m_MeasureWindow, SLOT(OnShowDistortionMap(bool)));

	m_MeasureWindow->GetToolBar()->addAction(ui.action_Enable_Measures);
	m_MeasureWindow->GetToolBar()->addAction(ui.action_Distortion_Map);
	m_MeasureWindow->GetToolBar()->addSeparator();
	m_MeasureWindow->GetToolBar()->addAction(ui.action_Select_Original);
	m_MeasureWindow->GetToolBar()->addAction(ui.action_Select_Processed_1);
	m_MeasureWindow->GetToolBar()->addAction(ui.action_Select_Processed_2);

	EnableButtons(0);
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
}

void MainWindow::on_action_Play_Pause_triggered(bool p)
{
	m_VideoViewList->GetControl()->PlayPause();
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
	// Open files
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

	// Pause
	PlaybackControl* control = m_VideoViewList->GetControl();
	PlaybackControl::Status status;
	control->GetStatus(&status);
	control->Play(false);

	for ( int i=0; i<fileList2.size(); i++) // if at least one QUrl is present in list
	{
		const QString& fName = fileList2[i];
		openFileInternal(fName);
	}

	// Update source list
	m_VideoViewList->GetProcessThread()->SetSources(m_VideoViewList->GetSourceIDList());
	
	// Trigger new seeking frame
	m_VideoViewList->GetControl()->Seek(status.lastProcessPTS, status.isPlaying);

	// Start source
	for (int i=0; i<m_VideoViewList->size(); ++i) 
	{
		VideoView* vv = m_VideoViewList->at(i);
		SourceThread* st = vv->GetSourceThread();
		if (st && !st->isRunning())
		{
			st->Start();
		}
	}
}


void MainWindow::on_action_Open_triggered()
{
	QSettings settings;
	QString openFilesPath = settings.value("main/openfilespath", "").toString();

	QFileDialog::Options options;
	QString selectedFilter;
	QStringList files = QFileDialog::getOpenFileNames(
		this, ("Open Video Files"),
		openFilesPath,
		("All Files (*);;YUVToolkit Script (YTS) Files (*.yts)"),
		&selectedFilter,
		options);

	if (files.size()>0)
	{
		QFileInfo fileInfo(files.first());
		settings.setValue("main/openfilespath", fileInfo.path());
	}

	openFiles(files);
}


void MainWindow::on_action_Run_Script_triggered()
{
	QSettings settings;
	QString openFilesPath = settings.value("main/openscriptpath", "").toString();

	QFileDialog::Options options;
	QString selectedFilter;
	QStringList fileList = QFileDialog::getOpenFileNames(
		this, ("QFileDialog::getOpenFileNames()"),
		openFilesPath,
		("YUVToolkit Script (YTS) Files (*.yts);;All Files (*)"),
		&selectedFilter,
		options);

	if (fileList.size()>0)
	{
		QFileInfo fileInfo(fileList.first());
		settings.setValue("main/openscriptpath", fileInfo.path());
	}

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
	QStringList lst;
	lst.append(strPath);
	openFiles(lst);
}

void MainWindow::openFileInternal( QString strPath)
{
	QStringList fileList;
	for (int i=0; i<m_VideoViewList->size(); ++i) 
	{
		SourceThread* st = m_VideoViewList->at(i)->GetSourceThread();
		if (st)
		{
			if (QFileInfo(st->GetSourcePath()) == QFileInfo(strPath))
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

	VideoView* vv = m_VideoViewList->NewVideoViewSource(strPath.toAscii());
}

void MainWindow::play( bool play )
{
	m_VideoViewList->GetControl()->Play(play);
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
	 engine.setProcessEventsInterval(50);

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
		resize(MINIMUM_WIDTH, MINIMUM_HEIGHT);
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
	QMainWindow* w = GetHostImpl()->NewMainWindow();
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
	// int ticks = qCeil(duration * fps / 1000)-1;

	if (m_Slider->maximum() != duration)
	{
		bool old = m_Slider->blockSignals(true);
		m_Slider->setRange(0, duration/SLIDER_STEP_MS);
		m_Slider->blockSignals(old);
	}

	// int pos = qRound(pts * fps / 1000);
	if (m_Slider->sliderPosition() != pts/SLIDER_STEP_MS)
	{
		bool old = m_Slider->blockSignals(true);
		m_Slider->setSliderPosition(pts/SLIDER_STEP_MS);
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

	SourceInfo info;
	longest->GetSource()->GetInfo(info);

	unsigned int pts = m_Slider->value()*SLIDER_STEP_MS;
	m_VideoViewList->GetControl()->Seek(pts);
}

void MainWindow::on_action_Step_Forward_triggered()
{
	int sinceLastStep = m_StepTime.restart();
	if (sinceLastStep<100)
	{
		stepVideo(5);
	}else
	{
		stepVideo(1);
	}
	
}

void MainWindow::on_action_Step_Back_triggered()
{
	int sinceLastStep = m_StepTime.restart();
	if (sinceLastStep<100)
	{
		stepVideo(-5);
	}else
	{
		stepVideo(-1);
	}
}

void MainWindow::on_action_Step_Forward_Fast_triggered()
{
	int sinceLastStep = m_StepTime.restart();
	if (sinceLastStep<100)
	{
		stepVideo(20);
	}else
	{
		stepVideo(10);
	}
}

void MainWindow::on_action_Step_Back_Fast_triggered()
{
	int sinceLastStep = m_StepTime.restart();
	if (sinceLastStep<100)
	{
		stepVideo(-20);
	}else
	{
		stepVideo(-10);
	}
}

void MainWindow::on_action_Seek_Beginning_triggered()
{
	m_VideoViewList->GetControl()->Seek(0, false);
}

void MainWindow::on_action_Seek_End_triggered()
{
	VideoView* longest = m_VideoViewList->longest();
	if (longest)
	{
		Source* source = longest->GetSource();
		
		SourceInfo info;
		source->GetInfo(info);

		m_VideoViewList->GetControl()->Seek(info.duration, false);
	}
}

void MainWindow::OnTimer()
{
	PlaybackControl::Status status;
	m_VideoViewList->GetControl()->GetStatus(&status);

	if (m_VideoViewList->size()>0)
	{
		m_VideoViewList->CheckResolutionChanged();
		m_VideoViewList->CheckRenderReset();
	}
	UpdateActiveVideoView();

	VideoView* active = m_ActiveVideoView;
	VideoView* longest = m_VideoViewList->longest();
	if (longest)
	{
		Source* source = VV_SOURCE(longest);
		FramePtr frame = VV_LASTFRAME(longest);
		
		if (source && frame)
		{
			SourceInfo info;
			source->GetInfo(info);

			OnUpdateSlider(info.duration, info.fps, status.lastProcessPTS);
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
		Source* source = VV_SOURCE(active);
		FramePtr frame = VV_LASTFRAME(active);

		if (source && frame)
		{
			SourceInfo info;
			source->GetInfo(info);

			str.clear();
			QTextStream(&str) << "" << frame->FrameNumber() << " / " << info.num_frames << "";
			m_TimeLabel1->setText(str);

			str.clear();
			QTextStream(&str) << frame->PTS() << " / " << info.duration << " ms";
			m_TimeLabel2->setText(str);			
		}

		if (status.isPlaying)
		{
			str.clear();
			float renderSpeed = m_VideoViewList->GetRenderThread()->GetSpeedRatio();
			QTextStream(&str) << QString("%1").arg(renderSpeed, 0, 'f', 2) << " x";
			m_RenderSpeedLabel->setText(str);
		}else
		{
			unsigned int pts = status.seekingPTS;
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

	if (status.isPlaying)
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

	ui.action_Compare->setChecked(m_MeasureDockWidget->toggleViewAction()->isChecked());
}

void MainWindow::stepVideo( int step )
{
	Source* source = 0;
	VideoView* longest = m_VideoViewList->longest();
	FramePtr lastFrame;
	if (longest)
	{
		source = longest->GetSource();
		lastFrame = longest->GetLastFrame();
	}

	if (!source || !lastFrame)
	{
		return;
	}

	SourceInfo info;
	source->GetInfo(info);

	int frame_num = MIN(MAX(((int)lastFrame->FrameNumber()) + step, 0), info.num_frames-1);
	unsigned int pts = source->IndexToPTS(frame_num);

	m_VideoViewList->GetControl()->Seek(pts, false);

	bool old = m_Slider->blockSignals(true);
	m_Slider->setSliderPosition(pts/SLIDER_STEP_MS);
	m_Slider->blockSignals(old);
}

void MainWindow::EnableButtons( int nrSources )
{
	ui.action_Play_Pause->setEnabled(nrSources!=0);
	ui.action_Step_Back->setEnabled(nrSources!=0);
	ui.action_Step_Back_Fast->setEnabled(nrSources!=0);
	ui.action_Step_Forward->setEnabled(nrSources!=0);
	ui.action_Step_Forward_Fast->setEnabled(nrSources!=0);
	ui.action_Seek_Beginning->setEnabled(nrSources!=0);
	ui.action_Seek_End->setEnabled(nrSources!=0);
	m_Slider->setEnabled(nrSources!=0);

	ui.action_Zoom_Switch->setEnabled(nrSources!=0);
	
	m_ActionsButton->setEnabled(nrSources!=0);
	ui.action_Compare->setEnabled(nrSources>1);
	ui.action_Select_Processed_2->setEnabled(nrSources>2);

	m_ZoomGroup->setEnabled(nrSources!=0);
	m_ColorGroup->setEnabled(nrSources!=0);

	if (nrSources == 0)
	{
		m_Slider->setSliderPosition(0);
	}

	if (nrSources<2)
	{
		m_MeasureDockWidget->hide();
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
	ui.action_Color->trigger();
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
	UpdateActiveVideoView();

	if (m_VideoViewList->size()==0 || m_ActiveVideoView == NULL)
	{
		QMenu menu(this);
		menu.addAction(ui.action_Open);
		menu.addAction(ui.action_New_Window);
		menu.addSeparator();
		menu.addAction(ui.action_About);
		menu.exec(event->globalPos());
	}else
	{
		QMenu* menu = m_ActiveVideoView->GetMenu();
		if (menu)
		{
			menu->exec(event->globalPos());
		}
	}	
}

void MainWindow::OnVideoViewClosed(VideoView* vv)
{
	m_ActiveVideoView = NULL;
	emit activeVideoViewChanged(NULL);

	if (vv->GetType() == PLUGIN_TRANSFORM)
	{
		// VideoQueue* vq = vv->GetRefVideoQueue();
		for (int i=0; i<m_VideoViewList->size(); i++)
		{
			VideoView* vvRef = m_VideoViewList->at(i);
			/*if (vq == vvRef->GetVideoQueue())
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
			}*/
		}
	}

	autoResizeWindow();	

	if (m_VideoViewList->size() == 0)
	{
		on_action_Clear_Selection_triggered();
	}
}

void MainWindow::OnVideoViewCreated( VideoView* vv)
{	
	vv->SetZoomLevel(m_ZoomMode);
}

void MainWindow::on_action_Quality_Measures_triggered()
{
	
}

void MainWindow::on_action_Homepage_triggered()
{
	QDesktopServices::openUrl(QUrl("http://www.yuvtoolkit.com", QUrl::TolerantMode));
}

void MainWindow::Init()
{
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
}

void MainWindow::OnColorActionTriggered( QAction* a )
{
	YUV_PLANE plane = PLANE_COLOR;
	if (a == ui.action_Y)
	{
		plane = PLANE_Y;
	}else if (a == ui.action_U)
	{
		plane = PLANE_U;
	}else if (a == ui.action_V)
	{
		plane = PLANE_V;
	}

	PlaybackControl::Status status;
	m_VideoViewList->GetControl()->GetStatus(&status);

	if (status.plane != plane)
	{
		m_VideoViewList->GetControl()->ShowPlane(plane);

		if (!status.isPlaying)
		{
			m_VideoViewList->GetControl()->Seek(status.lastDisplayPTS, false);
		}
	}
}

void MainWindow::on_action_Compare_triggered()
{
	m_MeasureDockWidget->toggleViewAction()->trigger();
}

void MainWindow::on_action_Select_From_triggered()
{
	m_VideoViewList->GetControl()->SelectFrom();

	updateSelectionSlider();
}

void MainWindow::on_action_Select_To_triggered()
{
	m_VideoViewList->GetControl()->SelectTo();

	updateSelectionSlider();

	
}

void MainWindow::on_action_Clear_Selection_triggered()
{
	m_VideoViewList->GetControl()->ClearSelection();
	updateSelectionSlider();
}

void MainWindow::updateSelectionSlider()
{
	PlaybackControl::Status status;
	m_VideoViewList->GetControl()->GetStatus(&status);

	if (status.selectionFrom == INVALID_PTS)
	{
		m_Slider->SetSelectionTo(-1);
		m_Slider->SetSelectionFrom(0);
	}else
	{
		m_Slider->SetSelectionFrom(status.selectionFrom/SLIDER_STEP_MS);
		if (status.selectionTo == INVALID_PTS)
		{
			m_Slider->SetSelectionTo(m_Slider->maximum());
		}else
		{
			m_Slider->SetSelectionTo(status.selectionTo/SLIDER_STEP_MS);
		}
	}
}

void MainWindow::OnVideoViewSourceListChanged()
{
	EnableButtons(m_VideoViewList->GetSourceIDList().size());
}

void MainWindow::OnVideoViewListChanged()
{
	ui.menu_Actions->clear();
	for (int i=0; i<m_VideoViewList->size(); i++)
	{
		VideoView* vv = m_VideoViewList->at(i);
		QMenu* menu = vv->GetMenu();
		ui.menu_Actions->addMenu(menu);
	}

	if (m_VideoViewList->size() == 1)
	{
		m_ActionsButton->setMenu(m_VideoViewList->at(0)->GetMenu());
	}else
	{
		m_ActionsButton->setMenu(ui.menu_Actions);
	}
}
