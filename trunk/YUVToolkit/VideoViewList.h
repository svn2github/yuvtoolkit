#ifndef VIDEO_VIEW_LIST_H
#define VIDEO_VIEW_LIST_H

#include "../PlugIns/YT_Interface.h"

class VideoView;
struct TransformActionData;
class RendererWidget;
class RenderThread;
class SourceThread;
class QMainWindow;
class QAction;

class VideoViewList : public QObject
{
	Q_OBJECT;
	RendererWidget* m_RenderWidget;
	QList<VideoView*> m_VideoList;
	QMainWindow* m_MainWindow;
public:
	VideoViewList(RendererWidget*);
	virtual ~VideoViewList();

	int size() const {return m_VideoList.size();}
	VideoView* at(int i) const {return m_VideoList.at(i);}
	VideoView* first() const {return m_VideoList.first();}
	VideoView* last() const {return m_VideoList.last();}
	VideoView* longest() const;

	VideoView* NewVideoView(QMainWindow*, const char* title);
	RenderThread* GetRenderThread() {return m_RenderThread;}

	void PlayAll();
	void PauseAll();
	void Seek(unsigned int pts);

	bool IsPlaying();

	unsigned int GetCurrentPTS() {return m_CurrentPTS; }
	bool GetRenderFrameList(QList<YT_Render_Frame>& list, unsigned int& pts, bool& carePTS); // carePTS set to false when seeking and ignore time stamp and time stamp difference
	unsigned int GetDuration() {return m_Duration;}
	unsigned int GetSeekingPTS() {return m_SeekingPTS; }
	void CheckRenderReset();
	void CheckLoopFromStart();
	void CheckResolutionChanged();
	void CheckSeeking();
		
public slots:
	void UpdateDuration();
	void CloseVideoView(VideoView*);
	void OnUpdateRenderWidgetPosition();
	void OnVideoViewTransformTriggered( QAction*, VideoView* , TransformActionData *);
signals:
	void ResolutionDurationChanged();
	void VideoViewCreated(VideoView*);
	void VideoViewClosed(VideoView*);
private:
	RenderThread* m_RenderThread;
	unsigned int m_Duration;
	unsigned int m_CurrentPTS;
	unsigned int m_VideoCount;
	VideoView* m_LongestVideoView;
	volatile bool m_Paused;

	volatile unsigned int  m_SeekingPTS;
	volatile unsigned int  m_SeekingPTSNext;
	volatile bool m_PlayAfterSeeking;

	QMutex m_MutexAddRemoveAndSeeking;
};

#endif