#ifndef RENDER_QUEUE_H
#define RENDER_QUEUE_H

#include "../PlugIns/YT_Interface.h"
#include "circularfifo.h"

#define RENDER_QUEUE_SIZE 4

class VideoQueue
{
public:
	struct Frame
	{
		YT_Frame_Ptr source; // frame pointer to the source 
		YT_Frame_Ptr render; // frame pointer to the render data
		unsigned int seekingPts; // PTS of seeking frame
	};

	VideoQueue(YT_Renderer* render);
	virtual ~VideoQueue();

	// receive new frame to push
	Frame* GetSourceFrame(YT_Format_Ptr format);
	void ReleaseSourceFrame(Frame* frame);

	unsigned int GetNextPTS(); // find the PTS of next frame in the queue
	Frame* GetRenderFrame(unsigned int PTS, bool seeking);  // Get frame to render
	Frame* GetLastRenderFrame();

	void ReleaseBuffers();
	void InitBuffers();

	static void RenderFrame(Frame*); // Copy frame from source to render
protected:
	YT_Renderer* m_Renderer;
	Frame* m_CurSourceFrame;

	QMutex m_MutexBufferAvailable;
	QWaitCondition m_BufferAvailable;

	QMutex m_MutexSourceReleased; // source frame released
	QWaitCondition m_SourceReleased;

	CircularFifo<Frame*, RENDER_QUEUE_SIZE> m_RenderQueue;
	CircularFifo<Frame*, RENDER_QUEUE_SIZE> m_EmptyQueue;
	Frame* m_LastRenderedFrame; // Buffer kept in here before push back to empty Queue
	// Frame* m_PrevToRenderFrame; // render frame keep here before pushed to RenderQueue
};

#endif // RENDER_QUEUE_H
