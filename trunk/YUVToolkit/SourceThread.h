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
	SourceThread(int id, PlaybackControl* c, const char* path);
	~SourceThread(void);

	void Start();
	void Stop();

	Source* GetSource() {return m_Source;}
	QString& GetSourcePath() {return m_Path;}

signals:
	void frameReady(FramePtr frame);

public slots:

private slots:
	void ReadFrames();

private:
	void run();
	
private:
	int m_ViewID;
	unsigned int m_LastSeekingPTS;
	QString m_Path;
	Source* m_Source;
	bool m_EndOfFile;
	
	FramePool* m_FramePool;
	PlaybackControl* m_Control;
	PlaybackControl::Status m_Status;
};

#endif