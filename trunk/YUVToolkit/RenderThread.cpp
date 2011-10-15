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

	for (int i = 0; i < m_RenderFrames.size(); i++) 
	{
		YT_Frame_Ptr& renderFrame = m_RenderFrames[i];

		if (renderFrame)
		{
			m_Renderer->Deallocate(renderFrame);
			renderFrame.clear();
		}
	}
	m_RenderFrames.clear();
 }

void RenderThread::Start()
{
	QThread::start();

	startTimer(8);
}

float RenderThread::GetSpeedRatio()
{
	return m_SpeedRatio;
}

void RenderThread::timerEvent( QTimerEvent *event )
{
	if (m_SceneQueue.size()==0)
	{
		return;
	}

	QList<YT_Frame_Ptr> newScene = m_SceneQueue.first();
	unsigned int renderPTS = m_PTSQueue.first();
	m_SceneQueue.removeFirst();
	m_PTSQueue.removeFirst();
	
	QList<YT_Frame_Ptr> renderFramesNew;
	for (int i = 0; i < newScene.size(); i++) 
	{
		YT_Frame_Ptr sourceFrame = newScene.at(i);
		if (!sourceFrame)
		{
			continue;
		}
		
		unsigned int viewID = sourceFrame->Info(VIEW_ID).toUInt();
		int j = m_ViewIDs.indexOf(viewID);
		if (j == -1)
		{
			continue;
		}		
		
		YT_Frame_Ptr renderFrame;		
		// Find existing render frame and extract it
		for (int k = 0; k < m_RenderFrames.size(); k++) 
		{
			YT_Frame_Ptr _frame = m_RenderFrames.at(k);
			if (_frame && _frame->Info(VIEW_ID).toUInt() == viewID)
			{
				m_RenderFrames.removeAt(i);
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

		renderFrame->SetInfo(SRC_RECT, m_SrcRects.at(j));
		renderFrame->SetInfo(DST_RECT, m_DstRects.at(j));
		renderFramesNew.append(renderFrame);
	}

	// Delete old frames
	for (int i=0; i<m_RenderFrames.size(); i++)
	{
		YT_Frame_Ptr renderFrame = m_RenderFrames.at(i);
		if (renderFrame)
		{
			m_Renderer->Deallocate(renderFrame);
		}
		renderFrame.clear();
	}
	m_RenderFrames.clear();

	// replace with new scene
	m_RenderFrames = renderFramesNew;
	m_Renderer->RenderScene(m_RenderFrames);
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

void RenderThread::Play( bool pause )
{

}

YT_Frame_Ptr RenderThread::ExtractFrame( QList<YT_Frame_Ptr>& lst, unsigned int id)
{
	for (int i = 0; i < lst.size(); i++) 
	{
		YT_Frame_Ptr frame = lst.at(i);
		if (frame && frame->Info(VIEW_ID).toUInt() == id)
		{
			lst.removeAt(i);
			return frame;
		}
	}

	return YT_Frame_Ptr();
}
