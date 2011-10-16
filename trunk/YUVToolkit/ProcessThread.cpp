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

	m_Frames.clear();
}

void ProcessThread::Start()
{
	m_Paused = false;
	m_SeekingPTS = INVALID_PTS;

	start();
	startTimer(15);
}

void ProcessThread::timerEvent( QTimerEvent *event )
{
	if (m_SeekingPTS != INVALID_PTS)
	{
		// Clean up queue tills seeking frame is found
		QMapIterator<unsigned int, QList<YT_Frame_Ptr> > i(m_Frames);
		while (i.hasNext()) 
		{
			i.next();

			unsigned int viewID = i.key();
			QList<YT_Frame_Ptr>& frameList = m_Frames[viewID];

			while (frameList.size()>0)
			{
				YT_Frame_Ptr frame = frameList.first();
				if (frame->Info(SEEKING_PTS).toUInt() != m_SeekingPTS)
				{
					frameList.removeFirst();
				}else
				{
					break;
				}
			}
		}

	}

	if (m_Paused && m_SeekingPTS == INVALID_PTS)
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
			emit sceneReady(scene, scene.first()->PTS(), scene.first()->Info(SEEKING_PTS).toUInt() != INVALID_PTS);

			if (m_SeekingPTS != INVALID_PTS && m_SeekingPTS == scene.first()->Info(SEEKING_PTS).toUInt())
			{
				m_SeekingPTS = INVALID_PTS;
				break; // don't keep processing after seeking
			}
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
	m_SeekingPTS = pts;
	m_Paused = !playAfterSeek;
}

bool ProcessThread::IsPlaying()
{
	return !m_Paused;
}
