#include "../PlugIns/YT_Interface.h"
#include "RenderThread.h"
#include "VideoQueue.h"
#include "VideoViewList.h"

#include <assert.h>

RenderThread::RenderThread(YT_Renderer* renderer, VideoViewList* list) : m_Renderer(renderer), 
	m_VideoViewList(list), exit(false), m_SpeedRatio(1.0f)
{
}

RenderThread::~RenderThread(void)
{

}

#define DIFF_PTS(x,y) qAbs<int>(((int)x)-((int)y))
void RenderThread::run()
{
	QList<YT_Render_Frame> frameList;
	QTime time;
	int renderFPS = 30;

	unsigned int lastPTS = INVALID_PTS;

	int elapsedSinceLastPTS = 0;
	while (!exit)
	{
		/// if PTS is same as previous one, reading is too slow, sleep for 10 ms
		// PTS time passed is smaller than actual time passed, reading cannot keep rendering speed, slow down rendering
		// else, sleep for the difference in PTS (converted to actual time)
		unsigned int pts = INVALID_PTS;
		bool shouldRender = m_VideoViewList->GetRenderFrameList(frameList, pts);

		if (!shouldRender)
		{
			continue;
		}

		m_Renderer->RenderScene(frameList);

		unsigned sleepTime = 0;
		if (pts<NEXT_PTS)
		{
			int diffPts = DIFF_PTS(pts, lastPTS);

			int elapsed = time.elapsed();
			if (pts>lastPTS && elapsed < diffPts)
			{
				sleepTime = diffPts - elapsed;
				if (sleepTime>1000)
				{
					WARNING_LOG("Render sleep time > 1000, pts %d lastPTS %d elapsed %d", pts, lastPTS, elapsed);

					sleepTime = 0;
				}
			}
		}else
		{
			sleepTime = 17;
		}

		int displayInterval = 16;
		if (sleepTime>displayInterval)
			msleep(sleepTime);

		// Compute render speed ratio
		if (pts != INVALID_PTS && lastPTS != INVALID_PTS)
		{
			int diffPts = DIFF_PTS(pts, lastPTS);
			elapsedSinceLastPTS += time.elapsed();

			if (diffPts > 0 && elapsedSinceLastPTS>0)
			{
				m_SpeedRatio = m_SpeedRatio + 0.1f * (diffPts*1.0f/ elapsedSinceLastPTS - m_SpeedRatio);

				elapsedSinceLastPTS = 0;
			}
		}

		lastPTS = pts;
		time.restart();
	}
	
}

void RenderThread::Stop()
{
	exit = true;
	wait();
 }

void RenderThread::Start()
{
	exit = false;
	QThread::start();
}

float RenderThread::GetSpeedRatio()
{
	return m_SpeedRatio;
}
