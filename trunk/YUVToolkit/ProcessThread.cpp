#include "ProcessThread.h"

ProcessThread::ProcessThread() : m_Paused(false)
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
	m_Paused = false;

	start();
	startTimer(15);
}

void ProcessThread::timerEvent( QTimerEvent *event )
{
	if (m_Paused)
	{
		return;
	}

	while (true)
	{
		QList<YT_Frame_Ptr> scene;
		QMapIterator<unsigned int, QList<YT_Frame_Ptr> > i(m_Frames);
		while (i.hasNext()) 
		{
			i.next();
		
			unsigned int viewID = i.key();
			QList<YT_Frame_Ptr>& frameList = m_Frames[viewID];

			if (frameList.size()>0)
			{
				YT_Frame_Ptr frame = frameList.first();
				frameList.removeFirst();

				scene.append(frame);
			}
		}

		if (scene.size()>0)
		{
			emit sceneReady(scene, scene.first()->PTS());
		}else
		{
			break;
		}
	}
}

void ProcessThread::ReceiveFrame( YT_Frame_Ptr frame )
{
	unsigned int viewID = frame->Info(VIEW_ID).toUInt();
	if (!m_Frames.contains(viewID))
	{
		m_Frames.insert(viewID, QList<YT_Frame_Ptr>());
	}
	m_Frames[viewID].append(frame);
}

void ProcessThread::Seek( unsigned int pts, bool playAfterSeek )
{
	m_Paused = !playAfterSeek;
}

bool ProcessThread::IsPlaying()
{
	return !m_Paused;
}
