#include "SourceThread.h"
#include "YT_InterfaceImpl.h"
#include "VideoView.h"
#include "ColorConversion.h"

#include <qstring.h>
#include <qmessagebox.h>
#include <QMutex>
#include <QMutexLocker>

SourceThread::SourceThread(VideoQueue* vq, const char* p) : m_Path(p), 
			m_Source(0), 
			m_ReadFramePTS(NEXT_PTS), m_VideoQueue(vq), m_EndOfFile(false)
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

void SourceThread::ReadFrame()
{
	if (m_EndOfFile)
	{
		return;
	}

	YT_Source_Info info;
	m_Source->GetInfo(info);

	VideoQueue::Frame* vqf = m_VideoQueue->GetSourceFrame(info.format);
	if (vqf && vqf->source && vqf->render)
	{
		vqf->shouldRender = true;
		vqf->isLastFrame = false;

		// Get next frame or seek 
		YT_RESULT res = m_Source->GetFrame(vqf->source, m_ReadFramePTS);

		if (res == YT_OK)
		{
			if (m_ReadFramePTS < NEXT_PTS)
			{
				m_ReadFramePTS = NEXT_PTS;
			}

			vqf->isLastFrame = (vqf->source->PTS() == info.lastPTS);

			VideoQueue::RenderFrame(vqf);
		}else
		{
			if (res == YT_END_OF_FILE)
			{
				INFO_LOG("m_Source->GetFrame returns YT_END_OF_FILE, seek %X", m_ReadFramePTS);
			}else
			{
				ERROR_LOG("m_Source->GetFrame returns error %d, seek %X", res, m_ReadFramePTS);
			}
			
			vqf->shouldRender = false;
			m_EndOfFile = true;
		}

		m_VideoQueue->ReleaseSourceFrame(vqf);
	}
}

void SourceThread::StopAndUninit()
{
	quit();
	wait();

	m_VideoQueue->ReleaseBuffers();

	if (m_Source)
	{
		m_Source->UnInit();
	}

	delete m_Source;

	m_Source = NULL;
}

void SourceThread::InitAndRun(unsigned int initialPTS)
{
	m_ReadFramePTS = initialPTS;
	m_Source->Init(m_Path);
	
	start();
}


void SourceThread::Seek(unsigned int pts)
{
	m_ReadFramePTS = pts;
	m_EndOfFile = false;

	ReadFrame();
}
