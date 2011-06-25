#ifndef RAWVIDEOTOOLKIT_H
#define RAWVIDEOTOOLKIT_H

#include <sigslot.h>

#include <QtGui>
// #include <QtScript>
#include "ui_MainWindow.h"

class VideoViewList;
class RendererWidget;
class YT_Renderer;
class SourceThread;
class VideoView;
class QPlainTextEdit;
class RenderThread;
struct YT_Graph_Stats;
struct YT_Source_Info;

class MainWindow : public QMainWindow, public sigslot::has_slots<>
{
	Q_OBJECT;
public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);
	virtual void closeEvent(QCloseEvent *event);

	void EnableButtons(bool enable);

signals:
	void activeVideoViewChanged(VideoView*);
	
public slots:
	void openFiles(const QStringList& fileList);
	void infoMsg(QString title, QString msg);
	void openFile(QString strPath);
	void play(bool play);

	void seekVideo(int pos); // change to pts
	void stepVideo(int step);
protected:
	void openScript(QString strPath, bool debug);
	void openFileInternal(QString strPath);
private:
	VideoViewList* m_VideoViewList;
	
	Ui::YT_MainWindow ui;
	QSlider* m_Slider;
	QLabel* m_TimeLabel1;
	QLabel* m_TimeLabel2;
	QLabel* m_RenderSpeedLabel;
	QLabel* m_ZoomLabel;
	int m_ZoomMode;
	unsigned int m_LastSliderValue;
	bool m_IsPlaying;
	QTimer* m_UpdateTimer;

	VideoView* m_ActiveVideoView;

	void UpdateStatusMessage(const QString& msg);
	void UpdateActiveVideoView();
	
	void autoResizeWindow();
	void SetZoomMode(int mode);

	static int windowCounter;
	QList<QAction*> m_RendererList;
	QString m_RenderType;	

public slots:
	void OnUpdateSlider(unsigned int duration, float fps, unsigned int pts);	
	void OnAutoResizeWindow();
	void OnActiveVideoViewChanged(VideoView*);
	void OnVideoViewClosed(VideoView* );
	void OnVideoViewCreated(VideoView* );
	
private slots:
	void OnTimer();
	void OnRendererSelected();

	void on_action_Play_Pause_triggered(bool play);
	
	void on_action_New_Window_triggered();
	void on_action_Open_triggered();
	void on_action_Run_Script_triggered();
	void on_action_Debug_Script_triggered();
	void on_action_Close_triggered();
	void on_action_About_triggered();
	void on_action_Exit_triggered();
	void on_action_File_Association();

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

	void on_actionShowLogging_triggered();
	void on_action_Enable_Logging_triggered();
};

#endif // RAWVIDEOTOOLKIT_H
