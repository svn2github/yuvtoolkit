#include "SourceThread.h"
#include "YT_InterfaceImpl.h"
#include "VideoView.h"
#include "ColorConversion.h"

#include <qstring.h>
#include <qmessagebox.h>
#include <QMutex>
#include <QMutexLocker>

SourceThread::SourceThread(VideoQueue* vq, const char* p) : m_State(READ_SOURCE), m_Path(p), 
			m_Source(0), 
			m_ReadFramePTS(NEXT_PTS), m_VideoQueue(vq)
{
	YT_PlugIn* plugin;
	plugin = GetHostImpl()->FindSourcePlugin(m_Path.right(4));
	m_Source = plugin->NewSource(m_Path.right(4));	
	m_Source->SeekTo.connect(this, &SourceThread::Seek);
}

SourceThread::~SourceThread(void)
{
}

void SourceThread::run()
{
	while (m_State != EXIT)
	{
		if (m_State == END_OF_FILE || m_ReadFramePTS == INVALID_PTS)
		{
			QMutexLocker locker(&m_MutexSeekOrResume);

			m_SeekOrResume.wait(&m_MutexSeekOrResume, 500);
			
			continue;
		}

		// INFO_LOG("Source thread State %X --		 %d", m_State, m_ReadFramePTS);
		if (m_State == READ_SOURCE)
		{
	YT_Source_Info info;
			m_Source->GetInfo(info);

			VideoQueue::Frame* vqf = m_VideoQueue->GetSourceFrame(info.format);
			if (vqf == NULL || vqf->source == NULL|| vqf->render == NULL )
			{
				msleep(10);
				continue;
			}else
			{
				QMutexLocker locker(&m_StateMutex);

				if (m_ReadFramePTS < NEXT_PTS)
				{
					INFO_LOG("Source thread seeks %d", m_ReadFramePTS);
					vqf->seekingPts = m_ReadFramePTS;
				}else
				{
					vqf->seekingPts = INVALID_PTS;
				}

				if (m_State == READ_SOURCE)
				{
					// Get next frame or seek 
					YT_RESULT res = m_Source->GetFrame(vqf->source, m_ReadFramePTS);

					if (res == YT_OK)
					{
						m_LastPTS = vqf->source->PTS();
						m_LastFrameNum = vqf->source->FrameNumber();
					
						if (m_ReadFramePTS < NEXT_PTS)
						{
							m_ReadFramePTS = INVALID_PTS;
						}

						// INFO_LOG("VideoQueue::RenderFrame %d - %d", vqf->source->PTS(), vqf->seekingPts);

						VideoQueue::RenderFrame(vqf);
					}else if (res == YT_END_OF_FILE)
					{
						INFO_LOG("m_Source->GetFrame returns YT_END_OF_FILE, seek %X", m_ReadFramePTS);
						m_State = END_OF_FILE;
					}else
					{	
						INFO_LOG("m_Source->GetFrame returns error %d, seek %X", res, m_ReadFramePTS);
					}
				}
			
				m_VideoQueue->ReleaseSourceFrame(vqf);
			}
		}
	}
}

void SourceThread::Stop()
{
	QMutexLocker locker(&m_StateMutex);
	m_State = EXIT;

	m_VideoQueue->ReleaseBuffers();

	m_SeekOrResume.wakeAll();

	wait();
	
	if (m_Source)
	{
		m_Source->UnInit();
	}

	delete m_Source;

	m_Source = NULL;
}

void SourceThread::Init()
{
	m_Source->Init(m_Path);
	
	start();
}


void SourceThread::Seek(unsigned int pts)
{
	INFO_LOG("SourceThread::Seek %d", pts);
	QMutexLocker locker(&m_StateMutex);

	m_State = READ_SOURCE;
	m_ReadFramePTS = pts;

	m_SeekOrResume.wakeAll();
}

void SourceThread::Pause()
{
	INFO_LOG("SourceThread::Pause %X", m_State);
	QMutexLocker locker(&m_StateMutex);

	m_State = READ_SOURCE;
	m_ReadFramePTS = INVALID_PTS;

	m_SeekOrResume.wakeAll();
}

void SourceThread::Play()
{
	QMutexLocker locker(&m_StateMutex);
	m_State = READ_SOURCE;
	m_ReadFramePTS = NEXT_PTS;

	m_SeekOrResume.wakeAll();
}

/*bool SourceThread::IsPlaying()
{
	return !m_Pause;
}*/

bool SourceThread::IsEndOfFile()
{
	return m_State == END_OF_FILE;
}


