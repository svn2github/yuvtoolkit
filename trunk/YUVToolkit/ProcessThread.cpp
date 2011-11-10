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
	m_LastPTS = INVALID_PTS;
	start();
}

void ProcessThread::ProcessFrameQueue()
{
	PlaybackControl::Status status;
	m_Control->GetStatus(&status);
		
	m_MutexSource.lock();
	UintList sourceViewIds = m_SourceViewIds;
	m_MutexSource.unlock();

	bool completed = CleanAndCheckQueue(sourceViewIds);	

	if (status.seekingPTS != INVALID_PTS)
	{
		bool allfound = true;
		FrameListPtr scene = FastSeekQueue(status.seekingPTS, sourceViewIds, allfound);
		
		if (allfound)
		{
			m_LastPTS = status.seekingPTS;
			m_Control->OnFrameProcessed(status.seekingPTS, status.seekingPTS);

			WARNING_LOG("ProcessThread seeking %d done", status.seekingPTS);
		}else
		{
			WARNING_LOG("ProcessThread seeking %d ... %d found", status.seekingPTS, scene->size());
		}

		emit sceneReady(scene, status.seekingPTS, true);

		return;
	}

	if (!status.isPlaying)
	{
		return;
	}

	if (!completed)
	{
		return;
	}

	while (true)
	{
		unsigned int ptsNext = 0;
		if (m_LastPTS == INVALID_PTS)
		{
			ptsNext = GetFirstPTS(sourceViewIds);
			if (ptsNext == INVALID_PTS)
			{
				return;
			}
		}else
		{
			ptsNext = GetNextPTS(sourceViewIds, m_LastPTS);
			if (ptsNext == INVALID_PTS)
			{
				return;
			}
			if (ptsNext-m_LastPTS<15)
			{
				ptsNext = m_LastPTS + 15;
			}
		}

		bool lastFrame = true;
		FrameListPtr scene; 
		QMapIterator<unsigned int, FrameList > i(m_SourceFrames);
		while (i.hasNext()) 
		{
			i.next();
		
			unsigned int viewID = i.key();
			FrameList& frameList = m_SourceFrames[viewID];

			FramePtr frameToRender;
			while (frameList.size()>0)
			{
				FramePtr frame = frameList.first();
				unsigned int _next = frame->Info(NEXT_PTS).toUInt();
				if (_next <= ptsNext)
				{
					frameList.removeFirst();
					continue;
				}

				frameToRender = frame;
				break;
			}

			if (frameToRender)
			{
				if (!frameToRender->Info(IS_LAST_FRAME).toBool())
				{
					lastFrame = false;
				}

				if (!scene)
				{
					scene = GetHostImpl()->GetFrameList();
				}
				scene->append(frameToRender);
			}else
			{
				// some frames missing
				return;
			}
		}

		if (scene && scene->size()>0)
		{
			m_Control->OnFrameProcessed(ptsNext, INVALID_PTS);
			emit sceneReady(scene, ptsNext, false);
			m_LastPTS = ptsNext;

			if (status.isPlaying)
			{
				if (status.selectionFrom != INVALID_PTS)
				{
					if (lastFrame || ptsNext>=status.selectionTo)
					{
						m_Control->Seek(status.selectionFrom);
					}
				}else if (lastFrame)
				{
					m_Control->Seek(0);
				}
			}
		}else
		{
			return;
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

bool ProcessThread::CleanAndCheckQueue(UintList& sourceViewIds)
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

	for (int i=0; i<sourceViewIds.size(); i++)
	{
		// if not all source has provided the frame
		if (!m_SourceFrames.contains(sourceViewIds.at(i)))
		{
			return false;
		}
	}
	return true;
}

FrameListPtr ProcessThread::FastSeekQueue( unsigned int pts, UintList sourceViewIds, bool& completed )
{
	// Clean up queue tills seeking frame is found, 
	// clean up so that source has buffer to fill-up
	// return list of seeking frame
	completed = true;
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
			completed = false;
		}
	}

	for (int i=0; i<sourceViewIds.size(); i++)
	{
		// if not all source has provided the frame
		if (!m_SourceFrames.contains(sourceViewIds.at(i)))
		{
			completed = false;
		}
	}

	return scene;
}

void ProcessThread::SetSources( UintList sourceViewIDs )
{
	QMutexLocker locker(&m_MutexSource);
	m_SourceViewIds = sourceViewIDs;
}

unsigned int ProcessThread::GetFirstPTS( UintList sourceViewIds )
{
	unsigned int ptsFirst = INVALID_PTS;
	QMapIterator<unsigned int, FrameList > i(m_SourceFrames);
	while (i.hasNext())
	{
		i.next();

		unsigned int viewID = i.key();
		FrameList& frameList = m_SourceFrames[viewID];

		if (frameList.size() == 0)
		{
			return INVALID_PTS;
		}

		FramePtr frame = frameList.first();
		unsigned int pts = frame->PTS();
		ptsFirst = qMin<unsigned int>(pts, ptsFirst);
	}
	return ptsFirst;
}

unsigned int ProcessThread::GetNextPTS( UintList sourceViewIds, unsigned int currentPTS )
{
	unsigned int ptsNext = INVALID_PTS;
	QMapIterator<unsigned int, FrameList > i(m_SourceFrames);
	while (i.hasNext())
	{
		i.next();

		unsigned int viewID = i.key();
		FrameList& frameList = m_SourceFrames[viewID];

		if (frameList.size() == 0)
		{
			return INVALID_PTS;
		}

		FramePtr frame = frameList.first();
		unsigned int pts = frame->PTS();
		if (pts<=currentPTS)
		{
			pts = frame->Info(NEXT_PTS).toUInt();
		}
		
		ptsNext = qMin<unsigned int>(pts, ptsNext);
	}
	return ptsNext;
}

void ProcessThread::SetMeasureRequests(const QList<MeasureRequest>& requests )
{
	QMutexLocker locker(&m_MutexMeasure);
	m_MeasureRequests = requests;
}

void ProcessThread::GetMeasureResults( QList<MeasureResult>& results )
{
	static int counter = 0;
	QMutexLocker locker(&m_MutexMeasure);
	for (int i=0; i<results.size(); i++)
	{
		MeasureResult& op = results[i];
		for (int i=0; i<PLANE_COUNT; i++)
		{
			op.results[i] = counter++;
		}
	}
}
