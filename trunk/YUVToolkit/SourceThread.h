#ifndef YT_GRAPH_H
#define YT_GRAPH_H

#include "YT_interface.h"
#include <QtGui>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "VideoQueue.h"

class QWidget;
class YT_Source;
class YT_Renderer;
class YT_Format;
class QDockWidget;
class VideoView;

#define BUFFER_COUNT 2

class SourceThread : public QThread, public sigslot::has_slots<>
{
	Q_OBJECT;
public:
	SourceThread(VideoQueue*, const char* path);
	~SourceThread(void);

	void Init();

	void Stop();

	bool IsEndOfFile();
	void Pause();
	void Play();

	void Seek(unsigned int pts);

	YT_Source* GetSource() {return m_Source;}
	QString& GetSourcePath() {return m_Path;}
private:
	typedef enum _STATE {
		READ_SOURCE,
		END_OF_FILE,
		EXIT,
	} STATE;
	volatile unsigned int m_ReadFramePTS;

	VideoQueue* m_VideoQueue;

	QMutex m_StateMutex;
	QMutex m_MutexSeekOrResume;
	QWaitCondition m_SeekOrResume;

	STATE m_State;

	void run();

	QString m_Path;
	QString m_RendererType;
	
	YT_Source* m_Source;
		
	volatile unsigned int m_LastSeekPTS;
	unsigned int m_LastPTS;
	unsigned int m_LastFrameNum;
};

#endif