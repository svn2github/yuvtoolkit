#include "SourceThread.h"
#include "YT_InterfaceImpl.h"
#include "VideoView.h"
#include "ColorConversion.h"

#include <qstring.h>
#include <qmessagebox.h>
#include <QMutex>
#include <QMutexLocker>

SourceThread::SourceThread(int id, PlaybackControl* c, const char* p) : m_Path(p), 
			m_Source(0), m_ViewID(id), m_EndOfFile(false), m_Control(c)
{
	moveToThread(this);

	YTPlugIn* plugin;
	plugin = GetHostImpl()->FindSourcePlugin(m_Path.right(4));
	m_Source = plugin->NewSource(m_Path.right(4));

	m_Source->Init(m_Path);

	m_FramePool = GetHostImpl()->NewFramePool(BUFFER_COUNT);
}

SourceThread::~SourceThread(void)
{
	if (m_Source)
	{
		m_Source->UnInit();
	}

	delete m_Source;
	m_Source = NULL;

	GetHostImpl()->ReleaseFramePool(m_FramePool);
}

void SourceThread::run()
{
	qRegisterMetaType<FramePtr>("FramePtr");
	qRegisterMetaType<FrameList>("FrameList");
	qRegisterMetaType<UintList>("UintList");
	qRegisterMetaType<RectList>("RectList");

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(ReadFrames()), Qt::DirectConnection);
	timer->start(15);

	exec();
}

void SourceThread::Stop()
{
	quit();
	wait();
}

void SourceThread::Start()
{
	start();
}


void SourceThread::ReadFrames()
{
	while (true)
	{
		m_Control->GetStatus(&m_Status);

		if (!m_FramePool->Size())
		{
			return;
		}

		if (m_EndOfFile && m_Status.seekingPTS == INVALID_PTS)
		{
			return;
		}

		SourceInfo info;
		m_Source->GetInfo(info);

		FramePtr frame = m_FramePool->Get();

		if (*frame->Format() != *info.format)
		{
			frame->Reset();
		}

		frame->SetFormat(info.format);
		frame->Allocate();
		frame->SetInfo(VIEW_ID, m_ViewID);
		frame->SetInfo(IS_LAST_FRAME, false);
		frame->SetInfo(SEEKING_PTS, INVALID_PTS);

		// Get next frame or seek 
		RESULT res = m_Source->GetFrame(frame, m_Status.seekingPTS);

		if (res == OK)
		{
			frame->SetInfo(IS_LAST_FRAME, frame->PTS() == info.lastPTS);
			frame->SetInfo(SEEKING_PTS, m_Status.seekingPTS);

			emit frameReady(frame);
			WARNING_LOG("FrameReady %d", frame->PTS());

			m_EndOfFile = false;
		}else
		{
			if (res == END_OF_FILE)
			{
				INFO_LOG("m_Source->GetFrame returns END_OF_FILE, seek %X", m_Status.seekingPTS);
			}else
			{
				ERROR_LOG("m_Source->GetFrame returns error %d, seek %X", res, m_Status.seekingPTS);
			}
			m_EndOfFile = true;
		}
	}	
}
