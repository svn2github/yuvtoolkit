#include "VideoViewList.h"
#include "VideoView.h"
#include "RendererWidget.h"
#include "RenderThread.h"
#include "VideoQueue.h"
#include "SourceThread.h"
#include "Layout.h"
#include "ColorConversion.h"

#include <assert.h>

VideoViewList::VideoViewList(RendererWidget* rw) : m_RenderThread(NULL), m_Duration(0), m_LongestVideoView(0), m_VideoCount(0), m_Paused(true), m_CurrentPTS(0), m_SeekingPTS(INVALID_PTS), m_SeekingPTSNext(INVALID_PTS), m_PlayAfterSeeking(false), m_EndOfFile(false), m_NeedSeekingRequest(false)
{
	m_RenderWidget = rw;
	
	connect(this, SIGNAL(ResolutionDurationChanged()), this, SLOT(UpdateDuration()));
}

VideoViewList::~VideoViewList()
{

}

VideoView* VideoViewList::NewVideoView( QMainWindow* win, const char* title )
{
	m_MainWindow = win;

	if (m_RenderWidget->GetRenderer() == NULL)
	{
		QSettings settings;
		QString renderType = settings.value("main/renderer", "D3D").toString();
		m_RenderWidget->Init(renderType);

		m_RenderThread = new RenderThread(m_RenderWidget->GetRenderer(), this);
	}

	m_RenderThread->Stop();

	VideoView* vv = new VideoView(win, m_RenderWidget);
	INFO_LOG("VideoViewList::NewVideoView %X", vv);

	{
		QMutexLocker locker(&m_MutexAddRemoveAndSeeking);

		m_RenderWidget->layout->AddView(vv);
		m_VideoList.append(vv);	
	}
	
	vv->SetTitle( title );

	OnUpdateRenderWidgetPosition();

	m_RenderThread->Start();

	connect(vv, SIGNAL(Close(VideoView*)), this, SLOT(CloseVideoView(VideoView*)));
	connect(vv, SIGNAL(TransformTriggered(QAction*, VideoView*, TransformActionData*)), this, SLOT(OnVideoViewTransformTriggered(QAction*, VideoView*, TransformActionData*)));

	emit VideoViewCreated(vv);

	return vv;
}

void VideoViewList::OnUpdateRenderWidgetPosition()
{
	QRect rcClient = m_RenderWidget->rect();
	int width = rcClient.width();
	m_RenderWidget->layout->UpdateGeometry();
}

void VideoViewList::CloseVideoView( VideoView* vv)
{
	INFO_LOG("VideoViewList::CloseVideoView %X", vv);
	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		VideoView* vv2 = m_VideoList.at(i);
		if (vv2->GetRefVideoQueue() == vv->GetVideoQueue())
		{
			CloseVideoView(vv2);
			i = 0; 
		}
	}

	m_RenderThread->Stop();
	
	vv->UnInit();
	{
		QMutexLocker locker(&m_MutexAddRemoveAndSeeking);

		m_RenderWidget->layout->RemoveView(vv);
		m_VideoList.removeOne(vv);		
	}
	
	UpdateDuration();

	if (m_VideoList.isEmpty())
	{
		m_RenderThread->Stop();
		SAFE_DELETE(m_RenderThread);

		m_RenderWidget->UnInit();
		m_RenderWidget->repaint();

		m_CurrentPTS = 0;
		m_SeekingPTS = m_SeekingPTSNext = INVALID_PTS;
	}else
	{
		m_RenderThread->Start();
	}

	emit VideoViewClosed(vv);

	SAFE_DELETE(vv);
}

void VideoViewList::CheckRenderReset()
{
	if (m_RenderThread && m_RenderWidget->GetRenderer()->NeedReset())
	{
		m_RenderThread->Stop();

		for (int i=0; i<m_VideoList.size(); ++i) 
		{
			VideoView* vv = m_VideoList.at(i);
			vv->GetVideoQueue()->ReleaseBuffers();
		}

		YT_RESULT res = m_RenderWidget->GetRenderer()->Reset();

		if (res == YT_OK)
		{
			for (int i=0; i<m_VideoList.size(); ++i) 
			{
				VideoView* vv = m_VideoList.at(i);
				vv->GetVideoQueue()->InitBuffers();
			}

			Seek(m_CurrentPTS, IsPlaying());

			m_RenderThread->Start();
		}
	}
}

bool VideoViewList::GetRenderFrameList( QList<YT_Render_Frame>& frameList, unsigned int& renderPTS )
{
	// INFO_LOG("VideoViewList::GetRenderFrameList");

	QMutexLocker locker(&m_MutexAddRemoveAndSeeking);

	while (frameList.size()>m_VideoList.size())
	{
		frameList.removeLast();
	}

	while (frameList.size() < m_VideoList.size())
	{
		YT_Render_Frame rf;
		frameList.append(rf);
	}


	unsigned int pts = NEXT_PTS; // Next pts to read from video queue
	// If paused, get the most recent item in the queue (flush queue)
	// If playing, find the closest frame in the future
	if (m_SeekingPTS != INVALID_PTS)
	{
		pts = m_SeekingPTS;
	}else if (m_Paused)
	{
		pts = m_CurrentPTS;
	}else
	{
		// Get next pts to render
		unsigned int minPTS = INVALID_PTS;
		unsigned int maxPTS = 0;
		for (int i=0; i<m_VideoList.size(); ++i) 
		{
			VideoView* vv = m_VideoList.at(i);
			if (vv->GetType() == YT_PLUGIN_SOURCE)
			{
				SourceThread* st = vv->GetSourceThread();
				VideoQueue* vq = vv->GetVideoQueue();
				
				bool isLastFrame = false;
				unsigned int currentPTS = vv->GetSource()->SeekPTS(m_CurrentPTS);
				unsigned int nextPts = vq->GetNextPTS(currentPTS, isLastFrame);
				if (nextPts!=INVALID_PTS &&  nextPts<minPTS && !isLastFrame)
				{
					minPTS = nextPts;
				}

				if (nextPts!=INVALID_PTS &&  nextPts>maxPTS)
				{
					maxPTS = nextPts;
				}
			}
		}
		if (minPTS < NEXT_PTS)
		{
			pts = minPTS;
		}else
		{
			pts = maxPTS;
		}
		assert(pts < NEXT_PTS);
	}

	m_CurrentPTS = pts;

	bool shouldRender = true;
	bool seekingDone = true;
	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		VideoView* vv = m_VideoList.at(i);
		if (!(vv->dstRect.isValid() && vv->srcRect.isValid()))
		{
			shouldRender = false;
		}

		if (vv->GetType() == YT_PLUGIN_UNKNOWN)
		{
			shouldRender = false;
		}else if (vv->GetType() == YT_PLUGIN_SOURCE)
		{
			VideoQueue::Frame* vqf = NULL;
			YT_Source* source = vv->GetSource();
			unsigned int sourcePTS = pts;
			
			if (source)
			{
 				sourcePTS = source->SeekPTS(pts);
			
				vqf = vv->GetVideoQueue()->GetRenderFrame(sourcePTS);
			}

			if (vqf)
			{
				YT_Render_Frame& rf = frameList[i];
				rf.frame = vqf->render;
				COPY_RECT(rf.srcRect, vv->srcRect);
				COPY_RECT(rf.dstRect, vv->dstRect);

				if (m_SeekingPTS != INVALID_PTS)
				{
					if (vqf->source->PTS() != sourcePTS)
					{
						seekingDone = false;
					}
				}
			}else
			{
				// Some frames are black, will cause flickering
				shouldRender = false;
				seekingDone = false;

				if (m_NeedSeekingRequest)
				{
					SourceThread* st = vv->GetSourceThread();
					QMetaObject::invokeMethod(st, "Seek", Qt::QueuedConnection, Q_ARG(unsigned int, sourcePTS));	
				}
			}
		}
	}

	m_NeedSeekingRequest = false;

	if (seekingDone && m_SeekingPTS != INVALID_PTS)
	{
		m_SeekingPTS = INVALID_PTS;
	}

	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		VideoView* vv = m_VideoList.at(i);
		if (vv->GetType() == YT_PLUGIN_TRANSFORM)
		{
			VideoQueue::Frame* ref  = vv->GetRefVideoQueue()->GetLastRenderFrame();			
			if (!ref)
			{
				continue;
			}

			YT_FormatImpl transformFormat;
			vv->GetTransform()->GetFormat(&(ref->source->Format()), vv->GetOutputName(), &transformFormat);
			
			VideoQueue::Frame* vqf = vv->GetVideoQueue()->GetSourceFrame(&(transformFormat));
			if (vqf != NULL && vqf->source != NULL && vqf->render != NULL )
			{
				QMap<QString, YT_Frame_Ptr> outputs;
				QMap<QString, QVariant> stats;
				outputs.insert(vv->GetOutputName(), vqf->source);
				vv->GetTransform()->Process(ref->source, outputs, stats);

				VideoQueue::RenderFrame(vqf);

				vv->GetVideoQueue()->ReleaseSourceFrame(vqf);

				/// Just to flush queue and make more source buffer available
				bool b;
				unsigned int ptsNext = vv->GetVideoQueue()->GetNextPTS(INVALID_PTS, b); // find the PTS of next frame in the queue

				YT_Render_Frame& rf = frameList[i];
				rf.frame = vqf->render;
				COPY_RECT(rf.srcRect, vv->srcRect);
				COPY_RECT(rf.dstRect, vv->dstRect);
			}
		}
	}

	if (m_Paused || m_SeekingPTS != INVALID_PTS)
	{
		renderPTS  = INVALID_PTS;
	}else
	{
		renderPTS  = pts;
	}

	return shouldRender;
}

class QThread3 : public QThread
{
public:
	static void msleep(unsigned long msecs) {
		QThread::msleep(msecs);
	}
};

void VideoViewList::Seek( unsigned int pts, bool playAfterSeek)
{
	QMutexLocker locker(&m_MutexAddRemoveAndSeeking);

	INFO_LOG("VideoViewList::Seek %d - m_SeekingPTS %d", pts, m_SeekingPTS);

	assert(m_VideoList.size()>0);

	m_Paused = true;
	m_PlayAfterSeeking = playAfterSeek;

	if (pts != INVALID_PTS)
	{
		if (m_SeekingPTS == INVALID_PTS)
		{
			m_SeekingPTS = pts;
			m_NeedSeekingRequest = true;

			/*
			for (int i=0; i<m_VideoList.size(); ++i) 
			{
			VideoView* vv = m_VideoList.at(i);
			VideoQueue* vq = vv->GetVideoQueue();
			SourceThread* st = vv->GetSourceThread();

			VideoQueue::Frame * frame = vq->GetLastRenderFrame();
			if (!frame || frame->source->PTS() != pts)
			{
			QMetaObject::invokeMethod(st, "Seek", Qt::QueuedConnection, Q_ARG(unsigned int, pts));
			}
			}*/
		}else
		{
			// Already seeking
			m_SeekingPTSNext = pts;
		}
	}
}

void VideoViewList::CheckLoopFromStart()
{
	if (m_SeekingPTS == INVALID_PTS && m_SeekingPTSNext == INVALID_PTS)
	{
		VideoView* vv = m_LongestVideoView;
		
		YT_Source* src = vv->GetSource();
		VideoQueue* queue = vv->GetVideoQueue();
		VideoQueue::Frame* lastFrame = queue->GetLastRenderFrame();

		YT_Source_Info info;
		src->GetInfo(info);

		bool endOfFile = (lastFrame && lastFrame->source->PTS() == info.lastPTS);

		if (!m_EndOfFile && endOfFile)
		{
			Seek(0, true);
		}

		m_EndOfFile = endOfFile;
	}
}

void VideoViewList::UpdateDuration()
{
	m_Duration = 0;
	m_LongestVideoView = NULL;
	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		VideoView* vv = m_VideoList.at(i);
		SourceThread* st = vv->GetSourceThread();
		if (st)
		{
			YT_Source* source = st->GetSource();

			YT_Source_Info info;
			source->GetInfo(info);

			if (info.duration>m_Duration)
			{
				m_Duration = info.duration;
				m_LongestVideoView = vv;
			}
		}
	}
}

void VideoViewList::CheckResolutionChanged()
{
	bool changed = false;
	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		if (m_VideoList.at(i)->CheckResolutionDurationChanged())
		{
			changed = true;
		}
	}

	if (changed)
	{
		emit ResolutionDurationChanged();
	}
}

void VideoViewList::OnVideoViewTransformTriggered( QAction* action, VideoView* vv, TransformActionData *data)
{
	bool hasView = false;
	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		VideoView* vv2 = m_VideoList.at(i);
		if (vv2->GetType() == YT_PLUGIN_TRANSFORM && vv2->GetRefVideoQueue() == 
			vv->GetVideoQueue() && vv2->GetOutputName() == data->outputName)
		{
			hasView = true;
			CloseVideoView(vv2);
		}
	}

	if (!hasView)
	{
		VideoView* planeVv = NewVideoView(m_MainWindow, data->outputName.toAscii());

		YT_Transform* transform = data->transformPlugin->NewTransform(data->transformName);

		planeVv->Init(transform, vv->GetVideoQueue(), data->outputName);
	}

	action->setChecked(!hasView);
}

void VideoViewList::CheckSeeking()
{
	if (m_SeekingPTS == INVALID_PTS && m_SeekingPTSNext != INVALID_PTS)
	{
		unsigned int pts = m_SeekingPTSNext;
		m_SeekingPTSNext = INVALID_PTS;

		Seek(pts, m_PlayAfterSeeking);
	}

	if (m_SeekingPTS == INVALID_PTS && m_SeekingPTSNext == INVALID_PTS)
	{
		if (m_PlayAfterSeeking)
		{
			m_Paused = false;
		}

		m_PlayAfterSeeking = false;
	}

}

bool VideoViewList::IsPlaying()
{
	return !m_Paused;
}

VideoView* VideoViewList::longest() const
{
	return m_LongestVideoView;
}

