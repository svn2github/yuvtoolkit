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
signals:
	void sceneRendered(FrameListPtr scene, unsigned int pts, bool seeking);

public slots:
	void RenderScene(FrameListPtr scene, unsigned int pts, bool seeking); 
	void SetLayout(UintList, RectList, RectList);

private slots:
	void Render();

protected:
	void run();
	FrameListPtr RenderFrames(FrameListPtr sourceFrames, FrameListPtr renderFramesOld);
	
	float m_SpeedRatio;
	Renderer* m_Renderer;
	PlaybackControl* m_Control;

	// render queue
	QList<FrameListPtr > m_SceneQueue;
	UintList m_PTSQueue;
	QList<bool> m_SeekingQueue;

	// Last rendered scene
	FrameListPtr m_LastSourceFrames;
	FrameListPtr m_LastRenderFrames;
	unsigned int m_LastPTS;
	bool m_LastSeeking;

	// Layout
	UintList m_ViewIDs;
	RectList m_SrcRects;
	RectList m_DstRects;

	QTime m_Timer, m_RenderSpeedTimer;
};
