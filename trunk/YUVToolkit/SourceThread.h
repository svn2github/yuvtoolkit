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

class SourceThread : public QThread
{
	Q_OBJECT;
public:
	SourceThread(VideoQueue*, const char* path);
	~SourceThread(void);

	void InitAndRun(unsigned int initialPTS);
	void StopAndUninit();

	YT_Source* GetSource() {return m_Source;}
	QString& GetSourcePath() {return m_Path;}

public slots:
	void ReadFrame();
	void Seek(unsigned int pts);
private:
	QString m_Path;
	YT_Source* m_Source;
	volatile unsigned int m_ReadFramePTS;
	volatile bool m_EndOfFile;
	VideoQueue* m_VideoQueue;

	void run();
};

#endif