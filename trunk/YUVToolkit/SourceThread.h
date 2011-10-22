#ifndef GRAPH_H
#define GRAPH_H

#include "YT_InterfaceImpl.h"
#include <QtGui>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class QWidget;
class Source;
class Renderer;
class Format;
class QDockWidget;
class VideoView;

#define BUFFER_COUNT 4

class SourceThread : public QThread
{
	Q_OBJECT;
public:
	SourceThread(int id, const char* path);
	~SourceThread(void);

	void Start(unsigned int initialPTS);
	void Stop();

	Source* GetSource() {return m_Source;}
	QString& GetSourcePath() {return m_Path;}

signals:
	void frameReady(FramePtr frame);

public slots:
	void Seek(unsigned int pts, bool playAfterSeek);

private slots:
	void ReadFrames();

private:
	void run();
	
private:
	int m_ViewID;
	QString m_Path;
	Source* m_Source;
	unsigned int m_SeekPTS;
	bool m_EndOfFile;
	
	FramePool* m_FramePool;
};

#endif