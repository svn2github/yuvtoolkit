#include "YT_Interface.h"
#include "RenderThread.h"

#include "VideoViewList.h"
#include "ColorConversion.h"

#include <assert.h>

RenderThread::RenderThread(YT_Renderer* renderer, VideoViewList* list) : m_Renderer(renderer), 
	m_VideoViewList(list), m_SpeedRatio(1.0f)
{
}

RenderThread::~RenderThread(void)
{

}

#define DIFF_PTS(x,y) qAbs<int>(((int)x)-((int)y))
void RenderThread::run()
{
	exec();
}

void RenderThread::Stop()
{
	quit();
	wait();

	for (int i = 0; i < m_LastRenderFrames.size(); i++) 
	{
		YT_Frame_Ptr& renderFrame = m_LastRenderFrames[i];

		if (renderFrame)
		{
			m_Renderer->Deallocate(renderFrame);
			renderFrame.clear();
		}
	}
	m_LastRenderFrames.clear();
 }

void RenderThread::Start()
{
	QThread::start();

	m_Timer.start();
	startTimer(8);
}

float RenderThread::GetSpeedRatio()
{
	return m_SpeedRatio;
}

void RenderThread::timerEvent( QTimerEvent *event )
{
	WARNING_LOG("Render start since last cycle: %d ms", m_Timer.restart());

	if (m_SceneQueue.size()>0)
	{		
		QList<YT_Frame_Ptr> newScene = m_SceneQueue.first();
		unsigned int renderPTS = m_PTSQueue.first();
		m_SceneQueue.removeFirst();
		m_PTSQueue.removeFirst();

		m_LastRenderFrames = RenderFrames(newScene, m_LastRenderFrames);
		m_LastSourceFrames = newScene;
	}

	if (m_LastRenderFrames.size()== 0 && m_LastSourceFrames.size()>0)
	{
		m_LastRenderFrames = RenderFrames(m_LastSourceFrames, m_LastRenderFrames);
	}

	// Create render scene with layout info
	QList<YT_Frame_Ptr> renderScene;
	for (int i=0; i<m_LastRenderFrames.size(); i++)
	{
		YT_Frame_Ptr renderFrame = m_LastRenderFrames.at(i);
		int j = m_ViewIDs.indexOf(renderFrame->Info(VIEW_ID).toUInt());
		if (j == -1)
		{
			continue;
		}

		renderFrame->SetInfo(SRC_RECT, m_SrcRects.at(j));
		renderFrame->SetInfo(DST_RECT, m_DstRects.at(j));
		renderScene.append(renderFrame);
	}

	WARNING_LOG("Render prepare scene took: %d ms", m_Timer.restart());
	m_Renderer->RenderScene(renderScene);
	WARNING_LOG("Render render scene took: %d ms", m_Timer.restart());

	emit sceneRendered(m_LastSourceFrames);
}

void RenderThread::RenderScene( QList<YT_Frame_Ptr> scene, unsigned int renderPTS )
{
	m_SceneQueue.append(scene);
	m_PTSQueue.append(renderPTS);
}

void RenderThread::SetLayout(QList<unsigned int> ids, QList<QRect> srcRects, QList<QRect> dstRects)
{
	m_ViewIDs = ids;
	m_SrcRects = srcRects;
	m_DstRects = dstRects;
}

QList<YT_Frame_Ptr> RenderThread::RenderFrames(QList<YT_Frame_Ptr> sourceFrames, QList<YT_Frame_Ptr> renderFramesOld)
{
	QList<YT_Frame_Ptr> renderFramesNew;
	for (int i = 0; i < sourceFrames.size(); i++) 
	{
		YT_Frame_Ptr sourceFrame = sourceFrames.at(i);
		if (!sourceFrame)
		{
			continue;
		}

		unsigned int viewID = sourceFrame->Info(VIEW_ID).toUInt();

		YT_Frame_Ptr renderFrame;		
		// Find existing render frame and extract it
		for (int k = 0; k < renderFramesOld.size(); k++) 
		{
			YT_Frame_Ptr _frame = renderFramesOld.at(k);
			if (_frame && _frame->Info(VIEW_ID).toUInt() == viewID)
			{
				renderFramesOld.removeAt(i);
				renderFrame = _frame;
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
		}

		// Render frame
		if (m_Renderer->GetFrame(renderFrame) == YT_OK)
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

		renderFramesNew.append(renderFrame);
	}

	// Delete old frames
	for (int i=0; i<renderFramesOld.size(); i++)
	{
		YT_Frame_Ptr renderFrame = renderFramesOld.at(i);
		if (renderFrame)
		{
			m_Renderer->Deallocate(renderFrame);
		}
		renderFrame.clear();
	}
	renderFramesOld.clear();

	return renderFramesNew;
}

