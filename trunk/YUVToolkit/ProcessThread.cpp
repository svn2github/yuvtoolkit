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

	m_SourceFrames.clear();
}

void ProcessThread::Start()
{
	start();
}

void ProcessThread::ProcessFrameQueue()
{
	PlaybackControl::Status status;
	m_Control->GetStatus(&status);
		
	m_Mutex.lock();
	UintList sourceViewIds = m_SourceViewIds;
	m_Mutex.unlock();

	CleanQueue(sourceViewIds);	

	if (status.seekingPTS != INVALID_PTS)
	{
		FrameListPtr scene = FastSeekQueue(status.seekingPTS, sourceViewIds);
		if (scene)
		{
			WARNING_LOG("ProcessThread seeking %d done", status.seekingPTS);

			m_Control->OnFrameProcessed(status.seekingPTS, status.seekingPTS);
			emit sceneReady(scene, status.seekingPTS, true);
		}else
		{
			WARNING_LOG("ProcessThread seeking %d not found", status.seekingPTS);
		}

		return;
	}

	if (!status.isPlaying)
	{
		return;
	}

	while (true)
	{
		bool lastFrame = true;
		FrameListPtr scene; 
		QMapIterator<unsigned int, FrameList > i(m_SourceFrames);
		while (i.hasNext()) 
		{
			i.next();
		
			unsigned int viewID = i.key();
			FrameList& frameList = m_SourceFrames[viewID];

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
	if (!m_SourceFrames.contains(viewID))
	{
		m_SourceFrames.insert(viewID, FrameList());
	}
	m_SourceFrames[viewID].append(frame);
}

void ProcessThread::CleanQueue(UintList& sourceViewIds)
{
	// Find views that doesn't exist any more and delete
	QMutableMapIterator<unsigned int, FrameList > i(m_SourceFrames);
	while (i.hasNext())
	{
		i.next();
		unsigned int viewID = i.key();
		if (sourceViewIds.indexOf(viewID) == -1)
		{
			i.remove();
		}
	}
}

FrameListPtr ProcessThread::FastSeekQueue( unsigned int pts, UintList sourceViewIds )
{
	// Clean up queue tills seeking frame is found, 
	// clean up so that source has buffer to fill-up
	// return list of seeking frame
	FrameListPtr scene = GetHostImpl()->GetFrameList();
	QMapIterator<unsigned int, FrameList > i(m_SourceFrames);
	while (i.hasNext())
	{
		i.next();

		unsigned int viewID = i.key();
		FrameList& frameList = m_SourceFrames[viewID];

		bool found = false;
		while (frameList.size()>0)
		{
			FramePtr frame = frameList.first();
			if (frame->Info(SEEKING_PTS).toUInt() != pts)
			{
				frameList.removeFirst();
			}else
			{
				found = true;
				scene->append(frame);
				break;
			}
		}

		if (!found)
		{
			return FrameListPtr();
		}
	}

	for (int i=0; i<sourceViewIds.size(); i++)
	{
		// if not all source has provided the frame
		if (!m_SourceFrames.contains(sourceViewIds.at(i)))
		{
			return FrameListPtr();
		}
	}

	return scene;
}

void ProcessThread::SetSources( UintList sourceViewIDs )
{
	QMutexLocker locker(&m_Mutex);
	m_SourceViewIds = sourceViewIDs;
}
