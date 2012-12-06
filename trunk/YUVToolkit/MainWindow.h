#ifndef RAWVIDEOTOOLKIT_H
#define RAWVIDEOTOOLKIT_H

#include <QtGui>
#include <Phonon/MediaSource>
#include <Phonon/MediaObject>
#include <Phonon/AudioOutput>

// #include <QtScript>
#include "ui_MainWindow.h"

class VideoViewList;
class RendererWidget;
class Renderer;
class SourceThread;
class VideoView;
class QPlainTextEdit;
class RenderThread;
class QClickableSlider;
struct Graph_Stats;
struct SourceInfo;
class MeasureWindow;
class ScoreWindow;
class QDockWidget;
class QScriptEngine;
class QScriptEngineDebugger;

class MainWindow : public QMainWindow
{
	Q_OBJECT;
public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);
	virtual void closeEvent(QCloseEvent *event);

	void EnableButtons(int nrSources);
	void Init();

signals:
	void activeVideoViewChanged(VideoView*);
	
public slots:
	QList<VideoView*> openFiles(const QStringList& fileList);
	void importExtension(QString strPath);
	void closeAll();
	void closeFile(VideoView* vv);
	void infoMsg(QString title, QString msg);
	VideoView* openFile(QString strPath);
	void play(bool play);
	void stepVideo(int step);

	void seekVideoFromSlider(); // change to pts
	void enableContextMenu(bool);
protected:
	VideoView* openFileInternal(QString strPath);
	void updateSelectionSlider();
private:
	void UpdateStatusMessage(const QString& msg);
	void UpdateActiveVideoView();
	
	void autoResizeWindow();
	void SetZoomMode(int mode);
public slots:
	void OnUpdateSlider(unsigned int duration, unsigned int pts);	
	void OnAutoResizeWindow();
	void OnActiveVideoViewChanged(VideoView*);
	void OnVideoViewClosed(VideoView* );
	void OnVideoViewCreated(VideoView* );
	void OnVideoViewListChanged();
	void OnVideoViewSourceListChanged();
	
private slots:
	void OnTimer();
	void OnRendererSelected();
	void openScript(QString strPath, bool debug);
	void on_action_Play_Pause_triggered(bool play);
	
	void on_action_New_Window_triggered();
	void on_action_Open_triggered();
	void on_action_Run_Script_triggered();
	void on_action_Debug_Script_triggered();
	void on_action_Close_triggered();
	void on_action_Homepage_triggered();
	void on_action_About_triggered();
	void on_action_Exit_triggered();
	
	void on_action_Zoom_50_triggered();
	void on_action_Zoom_100_triggered();
	void on_action_Zoom_200_triggered();
	void on_action_Zoom_400_triggered();
	void on_action_Zoom_Fit_triggered();
	void on_action_Zoom_Switch_triggered();

	void on_action_Step_Forward_triggered();
	void on_action_Step_Back_triggered();
	void on_action_Step_Forward_Fast_triggered();
	void on_action_Step_Back_Fast_triggered();
	void on_action_Seek_Beginning_triggered();
	void on_action_Seek_End_triggered();

	void on_actionShowLogging_triggered();
	void on_action_Enable_Logging_triggered();
	void on_action_Quality_Measures_triggered();
	void on_action_Compare_triggered();

	void on_action_Select_From_triggered();
	void on_action_Select_To_triggered();
	void on_action_Clear_Selection_triggered();
	
	void on_action_Options_triggered();
	void on_action_Enable_Measures_triggered();

	void OnColorActionTriggered(QAction* a);
private:
	VideoViewList* m_VideoViewList;
	VideoView* m_ActiveVideoView;

	Ui::MainWindow ui;
	QClickableSlider* m_Slider;
	QLabel* m_TimeLabel1;
	QLabel* m_TimeLabel2;
	QLabel* m_RenderSpeedLabel;
	QLabel* m_ZoomLabel;
	QToolButton* m_ActionsButton;
	QToolButton* m_CompareButton;
	QActionGroup* m_ColorGroup;
	QActionGroup* m_ZoomGroup;
	MeasureWindow* m_MeasureWindow;
	QDockWidget* m_MeasureDockWidget;

	ScoreWindow* m_ScoreWindow;
	QDockWidget* m_ScoreDockWidget;
	
	int m_ZoomMode;
	unsigned int m_LastSliderValue;
	bool m_IsPlaying;
	QTimer* m_UpdateTimer;
	QTime m_StepTime;


	static int windowCounter;
	QList<QAction*> m_RendererList;
	QString m_RenderType;

	QList<Phonon::MediaObject*> m;
	Phonon::AudioOutput a;
	bool m_AllowContextMenu;

	QScriptEngine* m_Engine;
	QScriptEngineDebugger *m_Debugger;
};

#endif // RAWVIDEOTOOLKIT_H
