#include "SourceThread.h"
#include "YT_InterfaceImpl.h"
#include "VideoView.h"
#include "ColorConversion.h"

#include <qstring.h>
#include <qmessagebox.h>
#include <QMutex>
#include <QMutexLocker>

SourceThread::SourceThread(int id, const char* p) : m_Path(p), 
			m_Source(0), m_ViewID(id), 
			m_SeekPTS(INVALID_PTS), m_EndOfFile(false), m_FramePool(BUFFER_COUNT)
{
	YT_PlugIn* plugin;
	plugin = GetHostImpl()->FindSourcePlugin(m_Path.right(4));
	m_Source = plugin->NewSource(m_Path.right(4));
}

SourceThread::~SourceThread(void)
{
}

void SourceThread::run()
{
	exec();
}

void SourceThread::Stop()
{
	quit();
	wait();

	if (m_Source)
	{
		m_Source->UnInit();
	}

	delete m_Source;
	m_Source = NULL;

	while (m_FramePool.Size() != BUFFER_COUNT)
	{
		WARNING_LOG("Waiting for frame pool to uninitialize (%s)... ", m_Path.toAscii());
		QThread::sleep(1);
	}
}

void SourceThread::Start(unsigned int initialPTS)
{
	m_SeekPTS = initialPTS;
	m_Source->Init(m_Path);
	
	start();
	startTimer(15);
}


void SourceThread::Seek(unsigned int pts, bool playAfterSeek)
{
	m_SeekPTS = pts;
	m_EndOfFile = false;
}

void SourceThread::timerEvent( QTimerEvent *event )
{
	while (true)
	{
		if (m_EndOfFile || !m_FramePool.Size())
		{
			return;
		}

		YT_Source_Info info;
		m_Source->GetInfo(info);

		YT_Frame_Ptr frame = m_FramePool.Get();

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
		YT_RESULT res = m_Source->GetFrame(frame, m_SeekPTS);

		if (res == YT_OK)
		{
			frame->SetInfo(IS_LAST_FRAME, frame->PTS() == info.lastPTS);
			frame->SetInfo(SEEKING_PTS, m_SeekPTS);

			emit frameReady(frame);
			WARNING_LOG("FrameReady %d", frame->PTS());

			if (m_SeekPTS < INVALID_PTS)
			{
				m_SeekPTS = INVALID_PTS;
			}

		}else
		{
			if (res == YT_END_OF_FILE)
			{
				INFO_LOG("m_Source->GetFrame returns YT_END_OF_FILE, seek %X", m_SeekPTS);
			}else
			{
				ERROR_LOG("m_Source->GetFrame returns error %d, seek %X", res, m_SeekPTS);
			}
			m_EndOfFile = true;
		}
	}	
}
