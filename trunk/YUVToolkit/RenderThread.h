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
protected:
	void run();
	bool exit;

	float m_SpeedRatio;
	YT_Renderer* m_Renderer;
	VideoViewList* m_VideoViewList;
};
