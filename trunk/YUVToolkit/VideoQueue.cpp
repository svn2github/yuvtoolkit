#include "VideoQueue.h"
#include <assert.h>
#include "ColorConversion.h"

#define RELEASE_SURFACE(rqf) if (rqf->surface) {rqf->surface->Release(); rqf->surface = NULL;}

VideoQueue::VideoQueue(YT_Renderer* render) :
	m_LastRenderedFrame(NULL), m_Renderer(render), m_RenderQueueLastPTS(INVALID_PTS)
{
	InitBuffers();
}

VideoQueue::~VideoQueue()
{
	ReleaseBuffers();
}

VideoQueue::Frame* VideoQueue::GetSourceFrame(YT_Format_Ptr format)
{
	unsigned int size = m_EmptyQueue.countItems();
	unsigned int size2 = m_RenderQueue.countItems();

	// INFO_LOG("VideoQueue::GetSourceFrame items %d - %d", size, size2);
	if (m_EmptyQueue.isEmpty())
	{
		// wake up by stop signal
		return NULL;
	}

	VideoQueue::Frame* rqf = 0;
	m_EmptyQueue.pop(rqf);
	assert(rqf!=0);

	if (rqf->source && rqf->source->Format() != *format)
	{
		if (m_Renderer->Deallocate(rqf->render) != YT_OK)
		{
			return NULL;
		}

		GetHost()->ReleaseFrame(rqf->source);
		rqf->source = NULL;
	}

	if (rqf->source == NULL)
	{
		rqf->source = GetHost()->NewFrame();
		rqf->source->SetFormat(*format);
		rqf->source->Allocate();

		if (m_Renderer->Allocate(rqf->render, format) != YT_OK)
		{
			return NULL;
		}
	}
 	
	if (m_Renderer->GetFrame(rqf->render) != YT_OK)
	{
		return NULL;
	}
	
	m_CurSourceFrame = rqf;
	return rqf;
}


void VideoQueue::ReleaseSourceFrame(VideoQueue::Frame* rqf)
{
	YT_RESULT res = m_Renderer->ReleaseFrame(rqf->render);
	assert (res == YT_OK);

	if (m_LastRenderedFrame)
	{
		// push to back of render queue
		m_RenderQueue.push(rqf);
	}else
	{
		m_LastRenderedFrame = rqf;
	}

	if (rqf->shouldRender)
	{
		m_RenderQueueLastPTS = rqf->source->PTS();
	}

	m_CurSourceFrame = NULL;

	WARNING_LOG("VideoQueue::ReleaseSourceFrame %d", m_RenderQueueLastPTS);

	m_SourceReleased.wakeAll();
}

unsigned int VideoQueue::GetNextPTS(unsigned int currentPTS, bool& isLastFrame)
{
	VideoQueue::Frame* rqfNext;

	DEBUG_LOG("VideoQueue size %d lastShown %d peekNext %d lastQueue %d", m_RenderQueue.countItems(), 
		(m_LastRenderedFrame)?m_LastRenderedFrame->source->PTS():INVALID_PTS, 
		(m_RenderQueue.peek(rqfNext))?rqfNext->source->PTS():INVALID_PTS, 
		m_RenderQueueLastPTS);

	// Remove all frames that are any of 1) should not render
	// 2) pts bigger than last item 3) smaller than currentPTS	
	while (m_RenderQueue.peek(rqfNext))
	{
		if ( (!rqfNext->shouldRender) || 
			(rqfNext->source->PTS()>m_RenderQueueLastPTS) ||
			(rqfNext->source->PTS() < currentPTS)) 
		{
			m_RenderQueue.pop(rqfNext);
			m_EmptyQueue.push(rqfNext);
		}else
		{
			break;
		}
	}

	if (!m_EmptyQueue.isEmpty())
	{
		emit BufferAvailable();
	}

	if (m_RenderQueue.peek(rqfNext))
	{
		isLastFrame = rqfNext->isLastFrame;

		DEBUG_LOG("VideoQueue::GetNextPTS next queue item %d", rqfNext->source->PTS());
		return rqfNext->source->PTS();
	}else
	{
		if (m_LastRenderedFrame && m_LastRenderedFrame->source->PTS() == currentPTS)
		{
			isLastFrame = m_LastRenderedFrame->isLastFrame;

			DEBUG_LOG("VideoQueue::GetNextPTS last shown item %d", currentPTS);
			return currentPTS;
		}else
		{
			DEBUG_LOG("VideoQueue::GetNextPTS return invalid");
			return INVALID_PTS;
		}
	}
}

VideoQueue::Frame* VideoQueue::GetRenderFrame(unsigned int PTS)
{
	if (m_LastRenderedFrame && m_LastRenderedFrame->source->PTS() == PTS)
	{
		if (!m_EmptyQueue.isEmpty())
		{
			emit BufferAvailable();
		}

		return m_LastRenderedFrame;
	}

	VideoQueue::Frame* rqfNext;
	while (m_RenderQueue.pop(rqfNext))
	{
		if (rqfNext->source->PTS() ==  PTS)
		{
			if (m_LastRenderedFrame)
			{
				m_EmptyQueue.push(m_LastRenderedFrame);
			}

			m_LastRenderedFrame = rqfNext;
			break;
		}else
		{
			rqfNext->shouldRender = false;
			m_EmptyQueue.push(rqfNext);
		}
	}

	if (!m_EmptyQueue.isEmpty())
	{
		emit BufferAvailable();
	}

	if (m_LastRenderedFrame && m_LastRenderedFrame->source->PTS() == PTS)
	{
		return m_LastRenderedFrame;
	}else
	{
		return NULL;
	}
}

VideoQueue::Frame* VideoQueue::GetLastRenderFrame()
{
	return m_LastRenderedFrame;
}

class QThread2 : public QThread
{
public:
	static void sleep(unsigned long secs) {
		QThread::sleep(secs);
	}
	static void msleep(unsigned long msecs) {
		QThread::msleep(msecs);
	}
	static void usleep(unsigned long usecs) {
		QThread::usleep(usecs);
	}
};


void VideoQueue::ReleaseBuffers()
{
	CircularFifo<Frame*, RENDER_QUEUE_SIZE> emptyQueue;

	while (!m_EmptyQueue.isEmpty())
	{
		VideoQueue::Frame* rqf = 0;
		m_EmptyQueue.pop(rqf);

		emptyQueue.push(rqf);
	}

	if (m_CurSourceFrame != NULL)
	{
		QMutexLocker locker(&m_MutexSourceReleased);
		m_SourceReleased.wait(&m_MutexSourceReleased);
	}

	while (!m_RenderQueue.isEmpty())
	{
		VideoQueue::Frame* rqf = 0;		
		m_RenderQueue.pop(rqf);
		
		m_Renderer->Deallocate(rqf->render);
		GetHost()->ReleaseFrame(rqf->source);
		
		delete rqf;
	}

	if (m_LastRenderedFrame)
	{
		VideoQueue::Frame* rqf = m_LastRenderedFrame;
		m_LastRenderedFrame = NULL;

		m_Renderer->Deallocate(rqf->render);
		GetHost()->ReleaseFrame(rqf->source);

		delete rqf;
	}

	while (!emptyQueue.isEmpty())
	{
		VideoQueue::Frame* rqf = 0;
		emptyQueue.pop(rqf);

		if (rqf->render)
		{
			m_Renderer->Deallocate(rqf->render);
		}

		if (rqf->source)
		{
			GetHost()->ReleaseFrame(rqf->source);
		}

		rqf->source = NULL;

		delete rqf;
	}

	emit BufferAvailable();
}

void VideoQueue::InitBuffers()
{
	for (int i=0; i<RENDER_QUEUE_SIZE; i++)
	{
		VideoQueue::Frame* rf = new VideoQueue::Frame;
		memset(rf, 0, sizeof(VideoQueue::Frame));

		m_EmptyQueue.push(rf);
	}

	emit BufferAvailable();
}

void VideoQueue::RenderFrame( Frame* vqf)
{
	YT_Frame_Ptr frame = vqf->source;
	YT_Frame_Ptr render = vqf->render;

	if (frame->Format() == render->Format())
	{
		for (int i=0; i<4; i++)
		{
			size_t len = render->Format().PlaneSize(i);
			if (len > 0)
			{
				memcpy(render->Data(i), frame->Data(i), len);
			}
		}

	}else
	{
		ColorConversion(*frame, *(render));
	}
}
