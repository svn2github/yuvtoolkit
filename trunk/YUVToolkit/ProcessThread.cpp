#include "ProcessThread.h"

ProcessThread::ProcessThread() : m_Play(true)
{
	moveToThread(this);
}

ProcessThread::~ProcessThread(void)
{
}

void ProcessThread::run()
{
	qRegisterMetaType<FramePtr>("FramePtr");
	qRegisterMetaType<FrameList>("FrameList");
	qRegisterMetaType<UintList>("UintList");
	qRegisterMetaType<RectList>("RectList");

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(ProcessFrameQueue()), Qt::DirectConnection);
	timer->start(15);

	exec();
}

void ProcessThread::Stop()
{
	quit();
	wait();

	m_Frames.clear();
}

void ProcessThread::Start(UintList sourceViewIDs)
{
	m_SourceViewIDs = sourceViewIDs;

	start();
}

void ProcessThread::ProcessFrameQueue()
{
	CleanQueue();

	if (!m_Play)
	{
		return;
	}

	/*if (!m_Play)
	{
		FrameList scene = FastSeekQueue(m_SeekingPTS);
		if (scene.size() == m_SourceViewIDs.size())
		{
			int siz = scene.size();
			emit sceneReady(scene, m_SeekingPTS, true);

			m_SeekingPTS = INVALID_PTS;
		}

		return;
	}*/
	while (true)
	{
		FrameList scene;
		QMapIterator<unsigned int, FrameList > i(m_Frames);
		while (i.hasNext()) 
		{
			i.next();
		
			unsigned int viewID = i.key();
			FrameList& frameList = m_Frames[viewID];

			if (frameList.size()>0)
			{
				FramePtr frame = frameList.first();
				frameList.removeFirst();

				scene.append(frame);
			}
		}

		if (scene.size()>0)
		{
			emit sceneReady(scene, scene.first()->PTS(), false);
		}else
		{
			break;
		}
	}
}

void ProcessThread::ReceiveFrame( FramePtr frame )
{
	unsigned int viewID = frame->Info(VIEW_ID).toUInt();
	if (!m_Frames.contains(viewID))
	{
		m_Frames.insert(viewID, FrameList());
	}
	m_Frames[viewID].append(frame);
}

void ProcessThread::Play( bool play )
{
	m_Play = play;
}

bool ProcessThread::IsPlaying()
{
	return m_Play;
}

void ProcessThread::CleanQueue()
{
	// Find views that doesn't exist any more and delete
	QMutableMapIterator<unsigned int, FrameList > i(m_Frames);
	while (i.hasNext())
	{
		i.next();

		unsigned int viewID = i.key();
		if (m_SourceViewIDs.indexOf(viewID) == -1)
		{
			i.remove();
		}
	}
}

FrameList ProcessThread::FastSeekQueue( unsigned int pts )
{
	// Clean up queue tills seeking frame is found, 
	// clean up so that source has buffer to fill-up
	// return list of seeking frame
	FrameList scene;
	QMapIterator<unsigned int, FrameList > i(m_Frames);
	while (i.hasNext())
	{
		i.next();

		unsigned int viewID = i.key();
		FrameList& frameList = m_Frames[viewID];

		while (frameList.size()>0)
		{
			FramePtr frame = frameList.first();
			if (frame->Info(SEEKING_PTS).toUInt() == INVALID_PTS)
			{
				frameList.removeFirst();
			}else
			{
				scene.append(frame);
				break;
			}
		}
	}

	return scene;
}
