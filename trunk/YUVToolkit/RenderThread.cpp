#include "YT_Interface.h"
#include "RenderThread.h"

#include "VideoViewList.h"
#include "ColorConversion.h"

#include <assert.h>

RenderThread::RenderThread(Renderer* renderer) : m_Renderer(renderer), 
	m_SpeedRatio(1.0f), m_Exit(false)
{
	moveToThread(this);
}

RenderThread::~RenderThread(void)
{

}

#define DIFF_PTS(x,y) qAbs<int>(((int)x)-((int)y))
void RenderThread::run()
{
	qRegisterMetaType<FramePtr>("FramePtr");
	qRegisterMetaType<FrameList>("FrameList");
	qRegisterMetaType<FrameListPtr>("FrameListPtr");

	WARNING_LOG("Render run start");

	QEventLoop eventLoop;
	while (!m_Exit)
	{
		eventLoop.processEvents();
		Render();
	}

	WARNING_LOG("Render run cleaning up");

	for (int i = 0; i < m_RenderFrames.size(); i++) 
	{
		FramePtr renderFrame = m_RenderFrames.at(i);
		if (renderFrame)
		{
			m_Renderer->Deallocate(renderFrame);
		}
	}
	m_RenderFrames.clear();

	WARNING_LOG("Render run finish");
}

void RenderThread::Stop()
{
	WARNING_LOG("Render Stop");
	m_Exit = true;
	wait();

	WARNING_LOG("Render Stop - Done");
 }

void RenderThread::Start()
{
	WARNING_LOG("Render Start");

	m_LastPTS = INVALID_PTS;
	m_LastSeeking = false;
	m_Exit = false;

	m_RenderInterval = 16;
	m_RenderCounter = 0;

	QThread::start();

	m_RenderSpeedTime.start();
	m_RenderCycleTime.start();

	WARNING_LOG("Render Start - Done");
}

float RenderThread::GetSpeedRatio()
{
	return m_SpeedRatio;
}

void RenderThread::Render()
{
	m_RenderCounter ++;
	bool nextIsSeeking = false;
	unsigned int nextPTS = GetNextPTS(nextIsSeeking);
	int diffPts = 0;
	if (!nextIsSeeking && nextPTS != INVALID_PTS && m_LastPTS != INVALID_PTS && nextPTS>m_LastPTS)
	{
		diffPts = qAbs<int>(((int)nextPTS)-((int)m_LastPTS));
	}

	bool newFrame = (nextPTS != INVALID_PTS) && (nextIsSeeking || ( diffPts < m_RenderCounter*m_RenderInterval + m_RenderInterval/2 ));
	FrameListPtr nextScene;
	if (newFrame)
	{	
		nextPTS = m_PTSQueue.takeFirst();
		nextIsSeeking = m_SeekingQueue.takeFirst();
		nextScene = m_SceneQueue.takeFirst();

		RenderFrames(nextScene);
	}else if (m_RenderFrames.size()== 0 && m_LastSourceFrames && m_LastSourceFrames->size()>0)
	{
		// paused and render reset
		RenderFrames(m_LastSourceFrames);
	}

	if (m_RenderFrames.size()>0)
	{
		UpdateLayout();
		m_Renderer->RenderScene(m_RenderFrames);

		int elapsedSinceLastRenderCycle = m_RenderCycleTime.restart();
		WARNING_LOG("Render cycle: %d (%.2f) ms", elapsedSinceLastRenderCycle, m_RenderInterval);
		m_RenderInterval = m_RenderInterval + 0.1f * (elapsedSinceLastRenderCycle - m_RenderInterval);
	}

	if (newFrame)
	{
		// Compute render speed ratio
		int elapsedSinceLastPTS = m_RenderSpeedTime.restart();
		WARNING_LOG("TIME %d, %d", diffPts, elapsedSinceLastPTS);
		if (diffPts > 0 && elapsedSinceLastPTS>0 && diffPts <= 1000 && elapsedSinceLastPTS<=1000)
		{
			m_SpeedRatio = m_SpeedRatio + 0.1f * (diffPts*1.0f/ elapsedSinceLastPTS - m_SpeedRatio);
		}

		m_LastSourceFrames = nextScene;
		m_LastSeeking = nextIsSeeking;
		m_LastPTS = nextPTS;

		m_RenderCounter = 0;

		emit sceneRendered(nextScene, m_LastPTS, m_LastSeeking);
	}
}

void RenderThread::RenderScene( FrameListPtr scene, unsigned int pts, bool seeking )
{
	m_SceneQueue.append(scene);
	m_PTSQueue.append(pts);
	m_SeekingQueue.append(seeking);
}

void RenderThread::SetLayout(UintList ids, RectList srcRects, RectList dstRects)
{
	QMutexLocker locker(&m_MutexLayout);
	m_ViewIDs = ids;
	m_SrcRects = srcRects;
	m_DstRects = dstRects;
}

void RenderThread::RenderFrames(FrameListPtr sourceFrames)
{
	for (int i = 0; i < sourceFrames->size(); i++) 
	{
		FramePtr sourceFrame = sourceFrames->at(i);
		if (!sourceFrame)
		{
			continue;
		}

		unsigned int viewID = sourceFrame->Info(VIEW_ID).toUInt();
		int pos = -1;
		for (int k = 0; k < m_RenderFrames.size(); k++) 
		{
			FramePtr _frame = m_RenderFrames.at(k);
			if (_frame && _frame->Info(VIEW_ID).toUInt() == viewID)
			{
				pos = k;
				break;
			}
		}

		if (pos == -1)
		{
			pos = m_RenderFrames.size();
			m_RenderFrames.append(FramePtr());
		}

		FramePtr& renderFrame = m_RenderFrames[pos];
		// Deallocate if resolution changed
		if (renderFrame && (sourceFrame->Format()->Width() != 
			renderFrame->Format()->Width() || 
			sourceFrame->Format()->Height() != 
			renderFrame->Format()->Height()))
		{
			m_Renderer->Deallocate(renderFrame);
			renderFrame.clear();
		}

		// Allocate if needed
		if (!renderFrame)
		{
			m_Renderer->Allocate(renderFrame, sourceFrame->Format());
			renderFrame->SetInfo(VIEW_ID, viewID);
		}

		// Render frame
		if (m_Renderer->GetFrame(renderFrame) == OK)
		{
			if (sourceFrame->Format() == renderFrame->Format())
			{
				for (int i=0; i<4; i++)
				{
					size_t len = renderFrame->Format()->PlaneSize(i);
					if (len > 0)
					{
						memcpy(renderFrame->Data(i), sourceFrame->Data(i), len);
					}
				}

			}else
			{
				ColorConversion(*sourceFrame, *renderFrame);
			}

			m_Renderer->ReleaseFrame(renderFrame);
		}
	}
}

void RenderThread::UpdateLayout()
{
	QMutexLocker locker(&m_MutexLayout);

	// Find frames without layout info and delete them
	QMutableListIterator<FramePtr> i(m_RenderFrames);
	while (i.hasNext()) 
	{
		FramePtr renderFrame = i.next();
		if (!renderFrame)
		{
			i.remove();
			continue;
		}

		int j = m_ViewIDs.indexOf(renderFrame->Info(VIEW_ID).toUInt());
		if (j == -1)
		{
			m_Renderer->Deallocate(renderFrame);
			i.remove();
		}else
		{
			renderFrame->SetInfo(SRC_RECT, m_SrcRects.at(j));
			renderFrame->SetInfo(DST_RECT, m_DstRects.at(j));
		}
	}
}

unsigned int RenderThread::GetNextPTS( bool& isSeeking )
{
	 int pos = m_SeekingQueue.lastIndexOf(true);
	 while ((pos--) > 0)
	 {
		 m_SeekingQueue.removeFirst();
		 m_PTSQueue.removeFirst();
		 m_SceneQueue.removeFirst();
	 }

	 if (m_PTSQueue.isEmpty())
	 {
		 return INVALID_PTS;
	 }

	 isSeeking = m_SeekingQueue.first();
	 return m_PTSQueue.first();
}

