#include "ProcessThread.h"

ProcessThread::ProcessThread() : m_Play(true), m_LastPTS(INVALID_PTS), m_SeekingPTS(INVALID_PTS)
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
	qRegisterMetaType<FrameListPtr>("FrameListPtr");
	qRegisterMetaType<UintListPtr>("UintListPtr");
	qRegisterMetaType<RectListPtr>("RectListPtr");

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

	if (m_SeekingPTS != INVALID_PTS)
	{
		FrameListPtr scene = FastSeekQueue(m_SeekingPTS);
		if (scene && scene->size() == m_SourceViewIDs.size())
		{
			emit sceneReady(scene, m_SeekingPTS, true);

			m_SeekingPTS = INVALID_PTS;
		}
	}

	if (!m_Play)
	{
		return;
	}

	while (true)
	{
		FrameListPtr scene = FrameListPtr(new FrameList);
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

				scene->append(frame);
			}
		}

		if (scene->size()>0)
		{
			m_LastPTS = scene->first()->PTS();
			emit sceneReady(scene, m_LastPTS, false);
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

void ProcessThread::Seek( unsigned int pts )
{
	m_SeekingPTS = pts;
}

bool ProcessThread::IsPlaying()
{
	return m_Play;
}

unsigned int ProcessThread::LastPTS()
{
	return m_LastPTS;
}

unsigned int ProcessThread::SeekingPTS()
{
	return m_SeekingPTS;
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

FrameListPtr ProcessThread::FastSeekQueue( unsigned int pts )
{
	// Clean up queue tills seeking frame is found, 
	// clean up so that source has buffer to fill-up
	// return list of seeking frame
	FrameListPtr scene = FrameListPtr(new FrameList);
	QMapIterator<unsigned int, FrameList > i(m_Frames);
	while (i.hasNext())
	{
		i.next();

		unsigned int viewID = i.key();
		FrameList& frameList = m_Frames[viewID];

		while (frameList.size()>0)
		{
			FramePtr frame = frameList.first();
			if (frame->Info(SEEKING_PTS).toUInt() != m_SeekingPTS)
			{
				frameList.removeFirst();
			}else
			{
				scene->append(frame);
				break;
			}
		}
	}

	return scene;
}

