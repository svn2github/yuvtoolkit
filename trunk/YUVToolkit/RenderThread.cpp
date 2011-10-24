#include "YT_Interface.h"
#include "RenderThread.h"

#include "VideoViewList.h"
#include "ColorConversion.h"

#include <assert.h>

RenderThread::RenderThread(Renderer* renderer) : m_Renderer(renderer), 
	m_SpeedRatio(1.0f)
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
	qRegisterMetaType<UintList>("UintList");
	qRegisterMetaType<RectList>("RectList");
	qRegisterMetaType<FrameListPtr>("FrameListPtr");

	WARNING_LOG("Render run start");

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(Render()), Qt::DirectConnection);
	timer->start(16);

	exec();

	timer->stop();
	SAFE_DELETE(timer);
	WARNING_LOG("Render run cleaning up");

	if (m_LastRenderFrames)
	{
		for (int i = 0; i < m_LastRenderFrames->size(); i++) 
		{
			FramePtr renderFrame = m_LastRenderFrames->at(i);
			if (renderFrame)
			{
				m_Renderer->Deallocate(renderFrame);
			}
		}
		GetHostImpl()->ReleaseFrameList(m_LastRenderFrames);
	}

	WARNING_LOG("Render run finish");
}

void RenderThread::Stop()
{
	WARNING_LOG("Render Stop");
	quit();
	wait();

	WARNING_LOG("Render Stop - Done");
 }

void RenderThread::Start()
{
	WARNING_LOG("Render Start");

	m_LastPTS = INVALID_PTS;
	m_LastSeeking = false;

	QThread::start();

	m_RenderCycleTime.start();

	WARNING_LOG("Render Start - Done");
}

float RenderThread::GetSpeedRatio()
{
	return m_SpeedRatio;
}

void RenderThread::Render()
{
	QMutexLocker locker(&m_MutexLayout);
	
	WARNING_LOG("Render start since last cycle: %d ms", m_RenderCycleTime.restart());

	int diffPts = 0;
	if (m_SceneQueue.size()>0)
	{		
		FrameListPtr newScene = m_SceneQueue.first();
		unsigned int pts = m_PTSQueue.first();
		bool seeking = m_SeekingQueue.first();
		
		m_SceneQueue.removeFirst();
		m_PTSQueue.removeFirst();
		m_SeekingQueue.removeFirst();

		// Compute render speed ratio
		if (pts != INVALID_PTS && m_LastPTS != INVALID_PTS && pts>m_LastPTS)
		{
			diffPts = qAbs<int>(((int)pts)-((int)m_LastPTS));
		}

		FrameListPtr frameList = RenderFrames(newScene, m_LastRenderFrames);
		if (m_LastRenderFrames)
		{
			GetHostImpl()->ReleaseFrameList(m_LastRenderFrames);
		}
		m_LastRenderFrames = frameList;
		if (m_LastSourceFrames)
		{
			GetHostImpl()->ReleaseFrameList(m_LastSourceFrames);
		}
		m_LastSourceFrames = newScene;
		m_LastSeeking = seeking;
		m_LastPTS = pts;
	}

	if (!m_LastRenderFrames || !m_LastSourceFrames)
	{
		return;
	}

	if (m_LastRenderFrames->size()== 0 && m_LastSourceFrames->size()>0)
	{
		FrameListPtr frameList = RenderFrames(m_LastSourceFrames, m_LastRenderFrames);
		GetHostImpl()->ReleaseFrameList(m_LastRenderFrames);
		m_LastRenderFrames = frameList;

	}

	// Create render scene with layout info
	for (int i=0; i<m_LastRenderFrames->size(); i++)
	{
		FramePtr renderFrame = m_LastRenderFrames->at(i);
		int j = m_ViewIDs.indexOf(renderFrame->Info(VIEW_ID).toUInt());
		if (j == -1)
		{
			renderFrame->SetInfo(SRC_RECT, QRect());
			renderFrame->SetInfo(DST_RECT, QRect());
		}else
		{
			renderFrame->SetInfo(SRC_RECT, m_SrcRects.at(j));
			renderFrame->SetInfo(DST_RECT, m_DstRects.at(j));
		}
	}

	WARNING_LOG("Render prepare scene took: %d ms", m_RenderCycleTime.restart());
	if (m_LastRenderFrames->size()>0 && m_LastRenderFrames->size() == m_ViewIDs.size())
	{
		m_Renderer->RenderScene(m_LastRenderFrames);
	}
	
	WARNING_LOG("Render render scene took: %d ms", m_RenderCycleTime.restart());

	// Compute render speed ratio
	int elapsedSinceLastPTS = m_RenderSpeedTime.elapsed();
	if (diffPts > 0 && elapsedSinceLastPTS>0 && diffPts <= 1000 && elapsedSinceLastPTS<=1000)
	{
		WARNING_LOG("TIME %d, %d", diffPts, elapsedSinceLastPTS);
		m_SpeedRatio = m_SpeedRatio + 0.1f * (diffPts*1.0f/ elapsedSinceLastPTS - m_SpeedRatio);
	}
	m_RenderSpeedTime.start();
	
	if (m_LastSourceFrames)
	{
		FrameListPtr rendered = FrameListPtr(new FrameList);
		rendered->append(*m_LastSourceFrames);
		emit sceneRendered(rendered, m_LastPTS, m_LastSeeking);
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

FrameListPtr RenderThread::RenderFrames(FrameListPtr sourceFrames, FrameListPtr renderFramesOld)
{
	FrameListPtr renderFramesNew;
	for (int i = 0; i < sourceFrames->size(); i++) 
	{
		FramePtr sourceFrame = sourceFrames->at(i);
		if (!sourceFrame)
		{
			continue;
		}

		unsigned int viewID = sourceFrame->Info(VIEW_ID).toUInt();

		FramePtr renderFrame;		
		// Find existing render frame and extract it
		if (renderFramesOld)
		{
			for (int k = 0; k < renderFramesOld->size(); k++) 
			{
				FramePtr _frame = renderFramesOld->at(k);
				if (_frame && _frame->Info(VIEW_ID).toUInt() == viewID)
				{
					renderFramesOld->removeAt(k);
					renderFrame = _frame;
					break;
				}
			}
		}
		
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

		if (!renderFramesNew)
		{
			renderFramesNew = GetHostImpl()->NewFrameList();
		}

		renderFramesNew->append(renderFrame);
	}

	// Delete old frames
	if (renderFramesOld)
	{
		for (int i=0; i<renderFramesOld->size(); i++)
		{
			FramePtr renderFrame = renderFramesOld->at(i);
			if (renderFrame)
			{
				m_Renderer->Deallocate(renderFrame);
			}
			renderFrame.clear();
		}
		renderFramesOld.clear();
	}
	

	return renderFramesNew;
}

