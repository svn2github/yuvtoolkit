#pragma once

#include "YT_Interface.h"

class VideoViewList;

class RenderThread : public QThread
{
	Q_OBJECT;
public:
	
	RenderThread(Renderer* renderer, VideoViewList* list);
	~RenderThread(void);

	void Start();
	void Stop();

	float GetSpeedRatio();
signals:
	void sceneRendered(FrameList scene, unsigned int pts, bool seeking);

public slots:
	void RenderScene(FrameList scene, unsigned int pts, bool seeking); 
	void SetLayout(UintList, RectList, RectList);

private slots:
	void Render();

protected:
	void run();
	FrameList RenderFrames(FrameList sourceFrames, FrameList renderFramesOld);
	
	float m_SpeedRatio;
	Renderer* m_Renderer;
	VideoViewList* m_VideoViewList;

	// render queue
	QList<FrameList > m_SceneQueue;
	UintList m_PTSQueue;
	QList<bool> m_SeekingQueue;

	// Last rendered scene
	FrameList m_LastSourceFrames;
	FrameList m_LastRenderFrames;
	unsigned int m_LastPTS;
	bool m_LastSeeking;

	// Layout
	UintList m_ViewIDs;
	RectList m_SrcRects;
	RectList m_DstRects;

	QTime m_Timer, m_RenderSpeedTimer;
};
