#include "YT_Interface.h"
#include "RenderThread.h"

#include "VideoViewList.h"
#include "ColorConversion.h"

#include <assert.h>

RenderThread::RenderThread(Renderer* renderer, PlaybackControl* c) :
	m_RenderCounter(0), m_RenderInterval(0), m_SpeedRatio(1.0f),
	m_Renderer(renderer), m_LastPTS(0), m_LastSeeking(false), m_Control(c), m_Exit(false)

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
	PlaybackControl::Status status;
	m_Control->GetStatus(&status);

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

		RenderFrames(nextScene, status.plane);
	}else if (m_RenderFrames.size()== 0 && m_LastSourceFrames && m_LastSourceFrames->size()>0)
	{
		// paused and render reset
		RenderFrames(m_LastSourceFrames, status.plane);
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

		m_Control->OnFrameDisplayed(m_LastPTS, m_LastSeeking);
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

void RenderThread::RenderFrames(FrameListPtr sourceFrames, YUV_PLANE plane)
{
	FrameImpl tempFrame;

	for (int i = 0; i < sourceFrames->size(); i++) 
	{
		FramePtr sourceFrameOrig = sourceFrames->at(i);
		if (!sourceFrameOrig)
		{
			continue;
		}
		unsigned int viewID = sourceFrameOrig->Info(VIEW_ID).toUInt();

		Frame* sourceFrame = sourceFrameOrig.data();
		float scaleX = 1;
		float scaleY = 1;
		if (plane != PLANE_COLOR)
		{
			COLOR_FORMAT c = sourceFrameOrig->Format()->Color();
			if (c == I420 || c == I422 || c == I444)
			{
				sourceFrame = &tempFrame;

				FormatPtr format = sourceFrameOrig->Format();
				sourceFrame->Format()->SetColor(Y800);
				sourceFrame->Format()->SetWidth(format->PlaneWidth(plane));
				sourceFrame->Format()->SetHeight(format->PlaneHeight(plane));
				sourceFrame->Format()->SetStride(0, format->Stride(plane));
				sourceFrame->Format()->PlaneSize(0);


				sourceFrame->SetData(0, sourceFrameOrig->Data(plane));

				scaleX = ((float)format->PlaneWidth(plane))/format->Width();
				scaleY = ((float)format->PlaneHeight(plane))/format->Height();
			}
		}
		
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
		}
		
		renderFrame->SetInfo(VIEW_ID, viewID);
		renderFrame->SetInfo(RENDER_SRC_SCALE_X, scaleX);
		renderFrame->SetInfo(RENDER_SRC_SCALE_Y, scaleY);

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
			QRect srcRect = m_SrcRects.at(j);
			float scaleX = renderFrame->Info(RENDER_SRC_SCALE_X).toFloat();
			float scaleY = renderFrame->Info(RENDER_SRC_SCALE_Y).toFloat();
			srcRect.setLeft((int)(srcRect.left()*scaleX));
			srcRect.setRight((int)(srcRect.right()*scaleX));
			srcRect.setTop((int)(srcRect.top()*scaleY));
			srcRect.setBottom((int)(srcRect.bottom()*scaleY));

			renderFrame->SetInfo(SRC_RECT, srcRect);
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

