#include <assert.h>
#include "ProcessThread.h"
#include "ColorMap.h"

ProcessThread::ProcessThread(PlaybackControl* c) : m_Control(c), m_IsLastFrame(false), m_DistMapFramePool(NULL)
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

	m_DistMapFramePool = GetHostImpl()->NewFramePool(8);

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(ProcessFrameQueue()), Qt::DirectConnection);
	timer->start(15);

	exec();

	timer->stop();
	SAFE_DELETE(timer);

	GetHostImpl()->ReleaseFramePool(m_DistMapFramePool);
	m_DistMapFramePool = NULL;
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
	m_IsLastFrame = false;
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
			
			ProcessMeasures(scene, status.plane);

			m_IsLastFrame = IsLastScene(scene);
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
				if (m_IsLastFrame)
				{
					if (status.selectionFrom != INVALID_PTS)
					{
						m_Control->Seek(status.selectionFrom);
					}else
					{
						m_Control->Seek(0);
					}
				}
				return;
			}
			if (ptsNext-m_LastPTS<15)
			{
				ptsNext = m_LastPTS + 15;
			}
		}

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
			
			ProcessMeasures(scene, status.plane);

			emit sceneReady(scene, ptsNext, false);
			m_LastPTS = ptsNext;
			m_IsLastFrame = IsLastScene(scene);

			if (status.isPlaying)
			{
				if (status.selectionFrom != INVALID_PTS)
				{
					if (ptsNext>=status.selectionTo)
					{
						m_Control->Seek(status.selectionFrom);
					}
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

void ProcessThread::SetMeasureRequests(const QList<MeasureItem>& requests )
{
	QMutexLocker locker(&m_MutexMeasure);
	m_MeasureRequests = requests;
}

void ProcessThread::GetMeasureResults( QList<MeasureItem>& results )
{
	QMutexLocker locker(&m_MutexMeasure);
	for (int i=0; i<results.size(); i++)
	{
		MeasureItem& item = results[i];
		const MeasureItem& item2 = m_MeasureRequests[i];
		assert(item.plugin == item2.plugin &&
			item.measure == item2.measure &&
			item.viewId == item2.viewId &&
			item.sourceViewId1 == item2.sourceViewId1 &&
			item.sourceViewId2 == item2.sourceViewId2 &&
			item.showDistortionMap == item2.showDistortionMap &&
			item.op.measureName == item2.op.measureName);

		for (int i=0; i<PLANE_COUNT; i++)
		{
			item.op.results[i] = item2.op.results[i];
			item.op.hasResults[i] = item2.op.hasResults[i];
		}
	}
}

void ProcessThread::ProcessMeasures( FrameListPtr scene, YUV_PLANE plane )
{
	QMutexLocker locker(&m_MutexMeasure);

	if (!m_MeasureRequests.size())
	{
		return;
	}

	PlugInInfo* plugin = 0;
	Measure* measure = 0;
	unsigned int sourceViewId1 = 0;
	unsigned int sourceViewId2 = 0;
	QList<MeasureOperation*> operations;
	QList<unsigned int> viewIds;

	QMap<QString, double> measureUpperRange;
	QMap<QString, double> measureLowerRange;
	QMap<QString, bool> measureBiggerValueIsBetter;

	// Group the measure operations that have same 
	// plugin/measure pointers and same sources
	for (int i=0; i<m_MeasureRequests.size(); i++)
	{
		MeasureItem& item = m_MeasureRequests[i];

		if (item.plugin != plugin || item.measure != measure || 
			item.sourceViewId1 != sourceViewId1 || item.sourceViewId2 != sourceViewId2)
		{
			ProcessOperations(scene, plane, operations, viewIds,
				measure, sourceViewId1, sourceViewId2);

			plugin = item.plugin;
			measure = item.measure;
			sourceViewId1 = item.sourceViewId1;
			sourceViewId2 = item.sourceViewId2;
		}

		item.op.hasResults[PLANE_Y] = 
			item.op.hasResults[PLANE_U] = 
			item.op.hasResults[PLANE_V] = 
			item.op.hasResults[PLANE_COLOR] = 
			false;

		if (item.showDistortionMap)
		{
			if (!m_DistMaps.contains(item.viewId))
			{
				m_DistMaps.insert(item.viewId, DistMapPtr(new DistMap));
			}

			item.op.distMap = m_DistMaps[item.viewId];
		}
		operations.append(&item.op);
		viewIds.append(item.viewId);
	}

	ProcessOperations(scene, plane, operations, viewIds,
		measure, sourceViewId1, sourceViewId2);
}

FramePtr ProcessThread::FindFrame( FrameListPtr lst, unsigned int id)
{
	for (int i=0; i<lst->size(); i++)
	{
		const FramePtr& ptr = lst->at(i);
		if (ptr->Info(VIEW_ID) == id)
		{
			return ptr;
		}
	}
	return FramePtr();
}

bool ProcessThread::IsLastScene( FrameListPtr scene )
{
	for (int i=0; i<scene->size(); i++)
	{
		FramePtr frame = scene->at(i);
		if (!frame->Info(IS_LAST_FRAME).toBool())
		{
			return false;
		}
	}

	return true;
}

void ProcessThread::ProcessOperations(FrameListPtr scene, YUV_PLANE plane, 
	QList<MeasureOperation*>& operations, QList<unsigned int>& viewIds,
	Measure* measure, unsigned int sourceViewId1, unsigned int sourceViewId2)
{
	if (!operations.size())
	{
		return;
	}

	FramePtr f1 = FindFrame(scene, sourceViewId1);
	FramePtr f2 = FindFrame(scene, sourceViewId2);
	if (f1 && f2)
	{
		YUV_PLANE p = plane;
		const MeasureCapabilities& cap = measure->GetCapabilities();
		if (p == PLANE_COLOR && !cap.hasColorDistortionMap)
		{
			p = PLANE_Y;
		}
		measure->Process(f1, f2, p, operations);

		for (int j=0; j<operations.size(); j++)
		{
			MeasureOperation* op = operations.at(j);
			if (op->distMapWidth && op->distMapHeight)
			{
				const MeasureInfo& info = GetHostImpl()->GetMeasureInfo(op->measureName);
				FramePtr frame = m_DistMapFramePool->Get();

				CreateColorMap(frame, op->distMap, op->distMapWidth, op->distMapHeight, 
					info.upperRange, info.lowerRange, info.biggerValueIsBetter);

				frame->SetInfo(VIEW_ID, viewIds.at(j));
				frame->SetInfo(IS_LAST_FRAME, true);
				scene->append(frame);
			}
		}
	}
	operations.clear();
	viewIds.clear();
}
