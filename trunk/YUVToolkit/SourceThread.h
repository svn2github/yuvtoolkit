#ifndef YT_GRAPH_H
#define YT_GRAPH_H

#include "YT_InterfaceImpl.h"
#include <QtGui>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class QWidget;
class YT_Source;
class YT_Renderer;
class YT_Format;
class QDockWidget;
class VideoView;

#define BUFFER_COUNT 8

class SourceThread : public QThread
{
	Q_OBJECT;
public:
	SourceThread(int, const char* path);
	~SourceThread(void);

	void Start(unsigned int initialPTS);
	void Stop();

	YT_Source* GetSource() {return m_Source;}
	QString& GetSourcePath() {return m_Path;}

signals:
	void frameReady(YT_Frame_Ptr frame);

public slots:
	void Seek(unsigned int pts, bool playAfterSeek);

private:
	void run();
	void timerEvent(QTimerEvent *event);

private:
	int m_ViewID;
	QString m_Path;
	YT_Source* m_Source;
	unsigned int m_SeekPTS;
	bool m_EndOfFile;
	
	YT_FramePool m_FramePool;
};

#endif