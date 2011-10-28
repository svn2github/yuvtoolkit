#include "ProcessThread.h"

ProcessThread::ProcessThread(PlaybackControl* c) : m_Control(c)
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

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(ProcessFrameQueue()), Qt::DirectConnection);
	timer->start(15);

	exec();

	timer->stop();
	SAFE_DELETE(timer);
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

	m_Control->GetStatus(&m_Status);

	if (m_Status.seekingPTS != INVALID_PTS)
	{
		FrameListPtr scene = FastSeekQueue(m_Status.seekingPTS);
		if (scene && scene->size() == m_SourceViewIDs.size())
		{
			m_Control->OnFrameProcessed(m_Status.seekingPTS, m_Status.seekingPTS);
			emit sceneReady(scene, m_Status.seekingPTS, true);
		}
	}

	if (!m_Status.isPlaying)
	{
		return;
	}

	while (true)
	{
		bool lastFrame = true;
		FrameListPtr scene; 
		QMapIterator<unsigned int, FrameList > i(m_Frames);
		while (i.hasNext()) 
		{
			i.next();
		
			unsigned int viewID = i.key();
			FrameList& frameList = m_Frames[viewID];

			if (frameList.size()>0)
			{
				FramePtr frame = frameList.takeFirst();
				if (frame->Info(SEEKING_PTS).toUInt()!=INVALID_PTS || !frame->Info(IS_LAST_FRAME).toBool())
				{
					lastFrame = false;
				}
				
				if (!scene)
				{
					scene = GetHostImpl()->GetFrameList();
				}
				scene->append(frame);
			}
		}

		if (scene && scene->size()>0)
		{
			unsigned int pts = scene->first()->PTS();
			m_Control->OnFrameProcessed(pts, INVALID_PTS);
			emit sceneReady(scene, pts, false);

			if (lastFrame)
			{
				m_Control->Seek(0);
			}
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
			if (frame->Info(SEEKING_PTS).toUInt() != pts)
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