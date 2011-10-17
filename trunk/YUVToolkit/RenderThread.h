#pragma once

#include "YT_Interface.h"

class VideoViewList;

class RenderThread : public QThread
{
	Q_OBJECT;
public:
	
	RenderThread(YT_Renderer* renderer, VideoViewList* list);
	~RenderThread(void);

	void Start();
	void Stop();

	float GetSpeedRatio();
signals:
	void sceneRendered(YT_Frame_List scene, unsigned int pts, bool seeking);

public slots:
	void RenderScene(YT_Frame_List scene, unsigned int pts, bool seeking); 
	void SetLayout(UintList, RectList, RectList);

private slots:
	void Render();

protected:
	void run();
	YT_Frame_List RenderFrames(YT_Frame_List sourceFrames, YT_Frame_List renderFramesOld);
	
	float m_SpeedRatio;
	YT_Renderer* m_Renderer;
	VideoViewList* m_VideoViewList;

	// render queue
	QList<YT_Frame_List > m_SceneQueue;
	UintList m_PTSQueue;
	QList<bool> m_SeekingQueue;

	// Last rendered scene
	YT_Frame_List m_LastSourceFrames;
	YT_Frame_List m_LastRenderFrames;
	unsigned int m_LastPTS;
	bool m_LastSeeking;

	// Layout
	UintList m_ViewIDs;
	RectList m_SrcRects;
	RectList m_DstRects;

	QTime m_Timer, m_RenderSpeedTimer;
};
