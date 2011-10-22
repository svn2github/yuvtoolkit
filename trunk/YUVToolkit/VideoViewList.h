#ifndef VIDEO_VIEW_LIST_H
#define VIDEO_VIEW_LIST_H

#include "YT_Interface.h"
#include "RenderThread.h"
#include "ProcessThread.h"

class VideoView;
struct TransformActionData;
class RendererWidget;
class SourceThread;
class QMainWindow;
class QAction;
class MeasureWindow;
class QDockWidget;

class VideoViewList : public QObject
{
	Q_OBJECT;
	RendererWidget* m_RenderWidget;
	QList<VideoView*> m_VideoList;
	QMainWindow* m_MainWindow;
	QList<MeasureWindow*> m_MeasureWindowList;
	QList<QDockWidget*> m_DockWidgetList;
public:
	VideoViewList(QMainWindow*, RendererWidget*);
	virtual ~VideoViewList();

	int size() const {return m_VideoList.size();}
	VideoView* at(int i) const {return m_VideoList.at(i);}
	VideoView* first() const {return m_VideoList.first();}
	VideoView* last() const {return m_VideoList.last();}
	VideoView* longest() const;
	VideoView* find(unsigned int id) const;
	UintList GetSourceIDList() const;

	VideoView* NewVideoView(const char* title);
	RenderThread* GetRenderThread() {return m_RenderThread;}
	ProcessThread* GetProcessThread() {return m_ProcessThread;}

	void Seek(unsigned int pts);

	bool IsPlaying();

	//bool GetRenderFrameList(QList<Render_Frame>& list, unsigned int& pts);
	unsigned int GetDuration() {return m_Duration;}
	void CheckRenderReset();
	void CheckLoopFromStart();
	void CheckResolutionChanged();
	void UpdateMeasureWindows();
	const QList<QDockWidget*>& GetDockWidgetList() {return m_DockWidgetList;}

	void StopSources();
	void StartSources();
		
public slots:
	void UpdateDuration();
	void CloseVideoView(VideoView*);
	void OnUpdateRenderWidgetPosition();
	void OnVideoViewTransformTriggered( QAction*, VideoView* , TransformActionData *);
	void OnSceneRendered(FrameListPtr scene, unsigned int pts, bool seeking);
signals:
	void ResolutionDurationChanged();
	void VideoViewCreated(VideoView*);
	void VideoViewClosed(VideoView*);
	void layoutUpdated(UintList, RectList, RectList);
private:
	RenderThread* m_RenderThread;
	ProcessThread* m_ProcessThread;

	unsigned int m_Duration;
	unsigned int m_VideoCount;
	VideoView* m_LongestVideoView;
	bool m_EndOfFile;
};

#endif