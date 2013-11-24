#ifndef GRAPH_H
#define GRAPH_H

#include "YT_InterfaceImpl.h"
#include <QWidget>
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
	SourceThread(SourceCallback* cb, int id, PlaybackControl* c, const char* path);
	~SourceThread(void);

	void Start();
	void Stop();
	void ResetSource(); // restart thread to ensure that source generate new frame

	Source* GetSource() {return m_Source;}
	QString& GetSourcePath() {return m_Path;}

signals:
	void frameReady(FramePtr frame);
	void sourceReset();

public slots:

private slots:
	void ReadFrames();
	// void VideoFormatReset();

private:
	void run();
	void EnsureFrameFormat(FramePtr frame, FormatPtr format);
	
private:
	volatile int m_ViewID;
	unsigned int m_LastSeekingPTS;
	QString m_Path;
	Source* m_Source;
	volatile bool m_EndOfFile;
	
	FramePtr m_FrameOrig;
	FormatPtr m_FormatNew;
	FramePool* m_FramePool;
	PlaybackControl* m_Control;
	PlaybackControl::Status m_Status;

	bool m_SourceReset;
};

#endif