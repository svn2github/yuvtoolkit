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
	QList<YT_Frame_Ptr> scene;
	QMapIterator<unsigned int, QList<YT_Frame_Ptr> > i(m_Frames);
	while (i.hasNext()) 
	{
		i.next();
		
		unsigned int viewID = i.key();
		QList<YT_Frame_Ptr>& frameList = m_Frames[viewID];
		int j = m_ViewIDs.indexOf(viewID);
		if (j == -1)
		{
			continue;
		}

		if (frameList.size()>0)
		{
			YT_Frame_Ptr frame = frameList.first();
			frameList.removeFirst();

			frame->SetInfo(SRC_RECT, m_SrcRects.at(j));
			frame->SetInfo(DST_RECT, m_DstRects.at(j));

			scene.append(frame);
		}
	}

	if (scene.size()>0)
	{
		emit sceneReady(scene, scene.first()->PTS());
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

void ProcessThread::SetLayout(QList<unsigned int> ids, QList<QRect> srcRects, QList<QRect> dstRects)
{
	m_ViewIDs = ids;
	m_SrcRects = srcRects;
	m_DstRects = dstRects;
}