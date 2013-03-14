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
#include "ScoreWindow.h"
#include "Options.h"
#include "Settings.h"
#include "TextFile.h"

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
	QMainWindow(parent, flags), m_VideoViewList(0),
	m_ActiveVideoView(0), m_Slider(0),
	m_TimeLabel1(0), m_TimeLabel2(0), m_RenderSpeedLabel(0),
	m_ZoomLabel(0), m_ActionsButton(0), m_CompareButton(0),
	m_ColorGroup(0), m_ZoomGroup(0), m_MeasureWindow(0),
	m_MeasureDockWidget(0), m_ZoomMode(0), m_LastSliderValue(0),
	m_IsPlaying(false), m_UpdateTimer(0), m_AllowContextMenu(true), m_Engine(NULL), m_Debugger(NULL)
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
	m_RenderType = settings.SETTINGS_GET_RENDERER();

	m_ZoomGroup = new QActionGroup(this);
	m_ZoomGroup->addAction(ui.action_Zoom_50);
	m_ZoomGroup->addAction(ui.action_Zoom_100);
	m_ZoomGroup->addAction(ui.action_Zoom_200);
	m_ZoomGroup->addAction(ui.action_Zoom_400);
	m_ZoomGroup->addAction(ui.action_Zoom_Fit);

	m_ZoomMode = settings.SETTINGS_GET_ZOOM();
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
	
	bool showDistMap = settings.SETTINGS_GET_SHOW_DIST_MAP();
	ui.action_Distortion_Map->setChecked(showDistMap);

	m_MeasureWindow->GetToolBar()->addAction(ui.action_Enable_Measures);
	m_MeasureWindow->GetToolBar()->addAction(ui.action_Distortion_Map);
	m_MeasureWindow->GetToolBar()->addSeparator();
	m_MeasureWindow->GetToolBar()->addAction(ui.action_Select_Original);
	m_MeasureWindow->GetToolBar()->addAction(ui.action_Select_Processed_1);
	m_MeasureWindow->GetToolBar()->addAction(ui.action_Select_Processed_2);


	m_ScoreDockWidget = new QDockWidget("Score", this );
	m_ScoreDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
	m_ScoreDockWidget->setVisible(false);

	m_ScoreWindow = new ScoreWindow(m_ScoreDockWidget);
	m_ScoreDockWidget->setWidget(m_ScoreWindow);
	addDockWidget(Qt::BottomDockWidgetArea, m_ScoreDockWidget);

	EnableButtons(0);
}

MainWindow::~MainWindow()
{
	if (m_Debugger)
	{
		m_Debugger->detach();
		SAFE_DELETE(m_Debugger);
	}

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


QList<VideoView*> MainWindow::openFiles( const QStringList& fileList )
{
	QList<VideoView*> lst;
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
		return lst;
	}

	// Pause
	PlaybackControl* control = m_VideoViewList->GetControl();
	PlaybackControl::Status status;
	control->GetStatus(&status);
	control->Play(false);

	for ( int i=0; i<fileList2.size(); i++) // if at least one QUrl is present in list
	{
		const QString& fName = fileList2[i];
		VideoView* vv = openFileInternal(fName);
		lst.append(vv);
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

	return lst;
}

void MainWindow::on_action_Open_triggered()
{
	QSettings settings;
	QString openFilesPath = settings.SETTINGS_GET_FILE_PATH();

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
		settings.SETTINGS_SET_FILE_PATH(fileInfo.path());


		for ( int i=0; i<files.size(); i++) // if at least one QUrl is present in list
		{
			Phonon::MediaObject* s = new Phonon::MediaObject();
			s->setCurrentSource(Phonon::MediaSource(files[i]));
			m.append(s);
			Phonon::createPath(s, &a);

			s->play();
		}
	}

	openFiles(files);
}


void MainWindow::on_action_Run_Script_triggered()
{
	QSettings settings;
	QString openFilesPath = settings.SETTINGS_GET_SCRIPT_PATH();

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
		settings.SETTINGS_SET_SCRIPT_PATH(fileInfo.path());
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
		closeFile(vv);
	}else
	{
		QMainWindow::close();
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
		settings.SETTINGS_SET_ZOOM(m_ZoomMode);
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



VideoView* MainWindow::openFile( QString strPath)
{
	QStringList lst;
	lst.append(strPath);
	QList<VideoView*> vvLst = openFiles(lst);
	if (vvLst.count()>0) 
	{
		return vvLst[0];
	}
	else
	{
		return NULL;
	}
}

VideoView* MainWindow::openFileInternal( QString strPath )
{
	QStringList fileList;
	for (int i=0; i<m_VideoViewList->size(); ++i)
	{
		VideoView* vv = m_VideoViewList->at(i);
		SourceThread* st = vv->GetSourceThread();
		if (st)
		{
			if (QFileInfo(st->GetSourcePath()) == QFileInfo(strPath))
			{
				return vv;
			}
		}
	}

	QFile file(strPath);
	if (!file.exists())
	{
		QMessageBox::warning(this, "Error", "File does not exist.");
		return NULL;
	}

	return m_VideoViewList->NewVideoViewSource(strPath.toAscii());
}

void MainWindow::play( bool play )
{
	m_VideoViewList->GetControl()->Play(play);
}


void MainWindow::infoMsg( QString title, QString msg )
{
	QMessageBox::information(this, title, msg, QMessageBox::Ok);
}

Q_DECLARE_METATYPE(VideoView*);
Q_DECLARE_METATYPE(QList<VideoView*>);
Q_DECLARE_METATYPE(QList<unsigned int>);
// Q_DECLARE_METATYPE(TextFile);
// Q_SCRIPT_DECLARE_QMETAOBJECT(TextFile, QString);

QScriptValue VideoViewToScriptValue(QScriptEngine *engine, VideoView* const &in)
{ 
	return engine->newQObject(in); 
}

void VideoViewFromScriptValue(const QScriptValue &object, VideoView* &out)
{ 
	out = qobject_cast<VideoView*>(object.toQObject()); 
}

QScriptValue TextFileConstructor(QScriptContext*context,QScriptEngine*engine)
{
	QString filename = context->argument(0).toString();
	bool write = false;
	if (context->argumentCount()>1) {
		write = context->argument(1).toBool();
	}
	TextFile* file = new TextFile(filename, write);
	return engine->newQObject(file,QScriptEngine::ScriptOwnership);
}

void MainWindow::openScript( QString strPath, bool debug )
{
	if (m_Engine) {
		delete m_Engine;
		m_Engine = NULL;
	}
	
	m_Engine = new QScriptEngine(this);
	QMainWindow* debuggerWindow = NULL;

	m_Engine->setProcessEventsInterval(50);
	m_Engine->globalObject().setProperty("yt", m_Engine->newQObject(this));

	if (debug)
	{
		if (!m_Debugger)
		{
			m_Debugger = new QScriptEngineDebugger(this);
		}

		debuggerWindow = m_Debugger->standardWindow();
		debuggerWindow->setWindowModality(Qt::ApplicationModal);
		// debuggerWindow->resize(1280, 704);
		m_Debugger->attachTo(m_Engine);
		m_Debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
	}

	QScriptValue& scriptMainWindow = m_Engine->globalObject().property("yt");
	scriptMainWindow.setProperty("scoreWindow", m_Engine->newQObject(m_ScoreWindow));

	scriptMainWindow.setProperty("actionScore", m_Engine->newQObject(m_ScoreDockWidget->toggleViewAction()));
	scriptMainWindow.setProperty("actionCompare", m_Engine->newQObject(ui.action_Compare));
	scriptMainWindow.setProperty("actionOptions", m_Engine->newQObject(ui.action_Options));
	scriptMainWindow.setProperty("actionClose", m_Engine->newQObject(ui.action_Close));

	scriptMainWindow.setProperty("actionPlayPause", m_Engine->newQObject(ui.action_Play_Pause));
	scriptMainWindow.setProperty("actionStepBack", m_Engine->newQObject(ui.action_Step_Back));
	scriptMainWindow.setProperty("actionStepBackFast", m_Engine->newQObject(ui.action_Step_Back_Fast));
	scriptMainWindow.setProperty("actionStepForward", m_Engine->newQObject(ui.action_Step_Forward));
	scriptMainWindow.setProperty("actionStepForwardFast", m_Engine->newQObject(ui.action_Step_Forward_Fast));

	scriptMainWindow.setProperty("actionSeekBeginning", m_Engine->newQObject(ui.action_Seek_Beginning));
	scriptMainWindow.setProperty("actionSeekEnd", m_Engine->newQObject(ui.action_Seek_End));

	scriptMainWindow.setProperty("actionZoom100", m_Engine->newQObject(ui.action_Zoom_100));
	scriptMainWindow.setProperty("actionZoom200", m_Engine->newQObject(ui.action_Zoom_200));
	scriptMainWindow.setProperty("actionZoom400", m_Engine->newQObject(ui.action_Zoom_400));
	scriptMainWindow.setProperty("actionZoom50", m_Engine->newQObject(ui.action_Zoom_50));
	scriptMainWindow.setProperty("actionZoomFit", m_Engine->newQObject(ui.action_Zoom_Fit));

	scriptMainWindow.setProperty("actionY", m_Engine->newQObject(ui.action_Y));
	scriptMainWindow.setProperty("actionU", m_Engine->newQObject(ui.action_U));
	scriptMainWindow.setProperty("actionV", m_Engine->newQObject(ui.action_V));
	scriptMainWindow.setProperty("actionColor", m_Engine->newQObject(ui.action_Color));

	// Register VideoView type
	qScriptRegisterMetaType(m_Engine, VideoViewToScriptValue, VideoViewFromScriptValue);
	qScriptRegisterSequenceMetaType<QList<VideoView*> >(m_Engine);
	qScriptRegisterSequenceMetaType<QList<unsigned int> >(m_Engine);

	// Register TextFile
	m_Engine->globalObject().setProperty("TextFile", m_Engine->newQMetaObject(&QObject::staticMetaObject, m_Engine->newFunction(TextFileConstructor)));

	QString oldPath = QDir::currentPath();
	QFileInfo fileInfo(strPath);
	QDir::setCurrent(fileInfo.path());
	
	QFile file(strPath);
	if (file.size()>1000000)
	{
		QMessageBox::warning(this, "Error", "File is too big. Cannot open it as script.");
		return;
	}
	file.open(QIODevice::ReadOnly);
	QString contents = file.readAll();
	file.close();

	QScriptValue ret = m_Engine->evaluate(contents, strPath);

	if (ret.isError())
	{
		QMessageBox::warning(this, tr("Error evaluating script"), tr("Line %0: %1")
			.arg(ret.property("lineNumber").toInt32())
			.arg(ret.toString()));
	} 
}

void MainWindow::autoResizeWindow()
{
	QSize display;
	QSize window = ui.rendererWidget->size();

	QSettings settings;
	if (isMaximized() || isMinimized() || !settings.SETTINGS_GET_AUTO_RESIZE())
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
		<< "<BR><BR>Copyright David Zhao (C) 2009-2012"
		<< "<BR>All Rights Reserved";
	about.setText(str);

	about.setIconPixmap(logo.scaledToWidth(128));
	about.exec();
}

void MainWindow::OnUpdateSlider(unsigned int duration, unsigned int pts)
{
	if (m_Slider->maximum() != (int)duration/SLIDER_STEP_MS)
	{
		bool old = m_Slider->blockSignals(true);
		m_Slider->setRange(0, duration/SLIDER_STEP_MS);

		m_Slider->setTickInterval(1000/SLIDER_STEP_MS);

		m_Slider->blockSignals(old);
	}

	if (m_Slider->sliderPosition() != (int) pts/SLIDER_STEP_MS)
	{
		bool old = m_Slider->blockSignals(true);
		m_Slider->setSliderPosition(pts/SLIDER_STEP_MS);
		m_Slider->blockSignals(old);
	}
}

void MainWindow::seekVideoFromSlider()
{
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
	const QList<unsigned int>& tsLst = m_VideoViewList->GetMergedTimeStamps();
	if (tsLst.size())
	{
		m_VideoViewList->GetControl()->Seek(tsLst.first(), false);
	}
}

void MainWindow::on_action_Seek_End_triggered()
{
	const QList<unsigned int>& tsLst = m_VideoViewList->GetMergedTimeStamps();
	if (tsLst.size())
	{
		m_VideoViewList->GetControl()->Seek(tsLst.last(), false);
	}
}

void MainWindow::OnTimer()
{
	ui.action_Enable_Logging->setChecked(GetHostImpl()->IsLoggingEnabled());
	ui.action_Compare->setChecked(m_MeasureDockWidget->toggleViewAction()->isChecked());

	if (!m_VideoViewList->size())
	{
		m_TimeLabel1->setText("   ");
		m_TimeLabel2->setText("   ");
		m_RenderSpeedLabel->setText("   ");
		return;
	}

	PlaybackControl::Status status;
	m_VideoViewList->GetControl()->GetStatus(&status);
	const QList<unsigned int>& tsLst = m_VideoViewList->GetMergedTimeStamps();

	m_VideoViewList->CheckRenderReset();
	
	UpdateActiveVideoView();
	VideoView* active = m_ActiveVideoView;
	OnUpdateSlider(tsLst.last(), status.lastProcessPTS);

	QString str;
	if (active)
	{
		QTextStream(&str) << "[" << QString("%1").arg(active->GetID()) << "] - " << active->title();
		UpdateStatusMessage(str);

		Source* source = VV_SOURCE(active);
		FramePtr frame = VV_LASTFRAME(active);
		if (source && frame)
		{
			SourceInfo* info = active->GetSourceInfo();

			str.clear();
			QTextStream(&str) << "" << frame->FrameNumber() << " / " << info->num_frames << "";
			m_TimeLabel1->setText(str);

			str.clear();
			QTextStream(&str) << frame->PTS() << " / " << info->duration << " ms";
			m_TimeLabel2->setText(str);
		}
	}else
	{
		m_TimeLabel1->setText("   ");

		str.clear();
		QTextStream(&str) << status.lastDisplayPTS << " / " << m_VideoViewList->GetDuration() << " ms";
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

	
	
}

void MainWindow::stepVideo( int step )
{
	PlaybackControl::Status status;
	m_VideoViewList->GetControl()->GetStatus(&status);

	const QList<unsigned int>& tsLst = m_VideoViewList->GetMergedTimeStamps();
	// QList<unsigned int>::iterator i = qBinaryFind(timeStampList.begin(), timeStampList.end(), status.lastDisplayPTS);
	int frame_num=0;
	for (frame_num=0; frame_num<tsLst.size(); frame_num++)
	{
		if (status.lastDisplayPTS == tsLst.at(frame_num))
		{
			break;
		}
	}
	
	frame_num = MIN(MAX(frame_num + step, 0), tsLst.size()-1);
	unsigned int pts = tsLst.at(frame_num);
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
	
	// ui.action_Select_Processed_2->setEnabled(nrSources>2);
	ui.action_Select_Original->setEnabled(false);
	ui.action_Select_Processed_1->setEnabled(false);
	ui.action_Select_Processed_2->setEnabled(false);

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
		if (m_VideoViewList->size()>0)
		{
			m_VideoViewList->DestroyRenderer();
		}

		m_RenderType = action->objectName();

		QSettings settings;
		settings.SETTINGS_SET_RENDERER(m_RenderType);

		if (m_VideoViewList->size()>0)
		{
			m_VideoViewList->CreateRenderer();
			m_VideoViewList->OnUpdateRenderWidgetPosition();

			PlaybackControl::Status status;
			m_VideoViewList->GetControl()->GetStatus(&status);
			if (!status.isPlaying)
			{
				// need to force source to deliver a new frame to renderer
				// m_VideoViewList->GetControl()->Play(true);

				for (int i=0; i<m_VideoViewList->size(); i++)
				{
					VideoView* vv = m_VideoViewList->at(i);
					vv->GetSourceThread()->ResetSource();
				}

				PlaybackControl::Status status = {0};
				m_VideoViewList->GetControl()->GetStatus(&status);

				m_VideoViewList->GetControl()->Seek(status.lastDisplayPTS);
			}

		}
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
	if (!view)
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
	if (!m_AllowContextMenu)
	{
		return;
	}

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
			// VideoView* vvRef = m_VideoViewList->at(i);
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

void MainWindow::on_action_Options_triggered()
{
	Options* o = new Options(this);
	if (m_MeasureWindow->isVisible())
	{
		connect(o, SIGNAL(OptionChanged()), m_MeasureWindow, SLOT(OnOptionChanged()));
	}

	o->exec(0);
}

void MainWindow::on_action_Enable_Measures_triggered()
{
	Options* o = new Options(this);
	if (m_MeasureWindow->isVisible())
	{
		connect(o, SIGNAL(OptionChanged()), m_MeasureWindow, SLOT(OnOptionChanged()));
	}

	o->exec(1);
}

void MainWindow::closeAll()
{
	while (m_VideoViewList->size() > 0) 
	{
		closeFile(m_VideoViewList->last());
	}
}

void MainWindow::closeFile(VideoView* vv)
{
	m_UpdateTimer->stop();

	m_VideoViewList->CloseVideoView(vv);

	m_UpdateTimer->start();
}

void MainWindow::enableContextMenu( bool b)
{
	m_AllowContextMenu = b;
}

void MainWindow::importExtension( QString strPath )
{
	if (!m_Engine)
		return;

	QScriptValue ret = m_Engine->importExtension(strPath);
	if (ret.isError())
	{
		QMessageBox::warning(this, tr("Error importing extension"), tr("Line %0: %1")
			.arg(ret.property("lineNumber").toInt32())
			.arg(ret.toString()));
	} 

}
