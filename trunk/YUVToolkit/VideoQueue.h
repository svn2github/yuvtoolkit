#ifndef RENDER_QUEUE_H
#define RENDER_QUEUE_H

#include "../PlugIns/YT_Interface.h"
#include "circularfifo.h"

#define RENDER_QUEUE_SIZE 4

class VideoQueue : public QObject
{
	Q_OBJECT;
public:
	struct Frame
	{
		YT_Frame_Ptr source; // frame pointer to the source 
		YT_Frame_Ptr render; // frame pointer to the render data
		bool shouldRender;
		bool isLastFrame;      // Last frame of source
	};

	VideoQueue(YT_Renderer* render);
	virtual ~VideoQueue();

	Frame* GetSourceFrame(YT_Format_Ptr format); // receive new frame to push
	void ReleaseSourceFrame(Frame* frame); // Finished rendering of new frame
	static void RenderFrame(Frame*); // Copy frame from source to render

	unsigned int GetNextPTS(unsigned int currentPTS, bool& isLastFrame); // find the PTS of next frame in the queue
	Frame* GetRenderFrame(unsigned int PTS);  // Get frame to render
	Frame* GetLastRenderFrame();

	void ReleaseBuffers();
	void InitBuffers();
	
signals:
	void BufferAvailable();
	// void SeekPTS(unsigned int PTS);
protected:
	YT_Renderer* m_Renderer;
	Frame* m_CurSourceFrame;

	QMutex m_MutexSourceReleased; // source frame released
	QWaitCondition m_SourceReleased;

	unsigned int m_RenderQueueLastPTS; // PTS of last item in render queue
	CircularFifo<Frame*, RENDER_QUEUE_SIZE> m_RenderQueue;
	CircularFifo<Frame*, RENDER_QUEUE_SIZE> m_EmptyQueue;
	Frame* m_LastRenderedFrame; // Buffer kept in here before push back to empty Queue
	// Frame* m_PrevToRenderFrame; // render frame keep here before pushed to RenderQueue

//	QTime m_LastBufferAvailable;
};

#endif // RENDER_QUEUE_H
