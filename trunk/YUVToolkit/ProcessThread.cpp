#include "ProcessThread.h"

ProcessThread::ProcessThread()
{
}

ProcessThread::~ProcessThread(void)
{
}

void ProcessThread::run()
{
	exec();
}

void ProcessThread::Stop()
{
	quit();
	wait();
}

void ProcessThread::Start()
{
	start();
	startTimer(15);
}

void ProcessThread::timerEvent( QTimerEvent *event )
{
	while (m_Frames.size()>0)
	{
		YT_Frame_Ptr frame = m_Frames.first();
		m_Frames.removeFirst();
		
		QList<YT_Frame_Ptr> scene;
		scene.append(frame);

		emit sceneReady(scene, frame->PTS());
	}
}

void ProcessThread::ReceiveFrame( YT_Frame_Ptr frame )
{
	m_Frames.append(frame);
}
