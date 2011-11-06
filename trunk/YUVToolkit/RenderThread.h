#pragma once

#include "YT_InterfaceImpl.h"

class RenderThread : public QThread
{
	Q_OBJECT;
public:
	
	RenderThread(Renderer* renderer, PlaybackControl* c);
	~RenderThread(void);

	void Start();
	void Stop();

	float GetSpeedRatio();

	void SetLayout(UintList, RectList, RectList);

signals:
	void sceneRendered(FrameListPtr scene, unsigned int pts, bool seeking);

public slots:
	void RenderScene(FrameListPtr scene, unsigned int pts, bool seeking); 
	
private slots:
	void Render();

protected:
	void run();
	void RenderFrames(FrameListPtr sourceFrames, YUV_PLANE plane);

	void CleanRenderList();
	unsigned int GetNextPTS(bool& isSeeking);

	int m_RenderCounter;
	float m_RenderInterval;
	float m_SpeedRatio;
	Renderer* m_Renderer;

	// render queue
	QList<FrameListPtr > m_SceneQueue;
	UintList m_PTSQueue;
	QList<bool> m_SeekingQueue;

	// Last scene
	FrameListPtr m_LastSourceFrames;
	unsigned int m_LastPTS;
	bool m_LastSeeking;

	// Layout
	UintList m_ViewIDs;
	RectList m_SrcRects;
	RectList m_DstRects;

	FrameList m_RenderFrames;

	QTime m_RenderCycleTime, m_RenderSpeedTime;

	PlaybackControl* m_Control;
	QMutex m_MutexLayout;
	volatile bool m_Exit;
};
