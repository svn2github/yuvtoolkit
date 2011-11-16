#ifndef VIDEO_VIEW_LIST_H
#define VIDEO_VIEW_LIST_H

#include "YT_InterfaceImpl.h"
#include "RenderThread.h"
#include "ProcessThread.h"

class VideoView;
struct TransformActionData;
class RendererWidget;
class SourceThread;
class QMainWindow;
class QAction;


class VideoViewList : public QObject
{
	Q_OBJECT;
	RendererWidget* m_RenderWidget;
	QList<VideoView*> m_VideoList;
	QMainWindow* m_MainWindow;

	unsigned int m_IDCounter;
	PlaybackControl m_Control;
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

	RenderThread* GetRenderThread() {return m_RenderThread;}
	ProcessThread* GetProcessThread() {return m_ProcessThread;}
	PlaybackControl* GetControl() {return &m_Control;}

	//bool GetRenderFrameList(QList<Render_Frame>& list, unsigned int& pts);
	unsigned int GetDuration() {return m_Duration;}
	void CheckRenderReset();
	void CheckResolutionChanged();
	
	void UpdateRenderLayout();

	unsigned int NewVideoViewId();
	VideoView* NewVideoViewSource(const char* path);
	VideoView* NewVideoViewCompare(QString measureName, unsigned int viewId, unsigned int orig, unsigned int processed);

	void CloseVideoView(unsigned int viewId);
private:
	VideoView* NewVideoViewInternal(QString title, unsigned int viewId);
public slots:
	void UpdateDuration();
	void CloseVideoView(VideoView*);
	void OnUpdateRenderWidgetPosition();
	void OnVideoViewTransformTriggered( QAction*, VideoView* , TransformActionData *);
	void OnSceneRendered(FrameListPtr scene, unsigned int pts, bool seeking);
	void OnViewPortUpdated(VideoView*, double x, double y);
signals:
	void ResolutionDurationChanged();
	void VideoViewCreated(VideoView*);
	void VideoViewClosed(VideoView*);
	void VideoViewSourceListChanged();
	void VideoViewListChanged();
private:
	RenderThread* m_RenderThread;
	ProcessThread* m_ProcessThread;

	unsigned int m_Duration;
	unsigned int m_VideoCount;
	VideoView* m_LongestVideoView;
	bool m_EndOfFile;
};

#endif