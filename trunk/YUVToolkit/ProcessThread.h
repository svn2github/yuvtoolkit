#ifndef PROCESS_THREAD_H
#define PROCESS_THREAD_H

#include "YT_InterfaceImpl.h"
#include <QThread>
#include <QList>
#include <QMap>

class ProcessThread : public QThread
{
	Q_OBJECT;
public:
	ProcessThread();
	~ProcessThread();

	void Start();
	void Stop();

	bool IsPlaying();
signals:
	// Signals that one scene is ready for render
	void sceneReady(QList<YT_Frame_Ptr> scene, unsigned int renderPTS);

	// Signals that seeking is done for all source views
	void seekDone(unsigned int pts);

	// Signals (to source) to seek
	void sourceSeek(unsigned int pts);
	
public slots:
	void ReceiveFrame(YT_Frame_Ptr frame);
	
	// Seek, and play/pause after seeking
	// pts can be INVALID_PTS to just keep current PTS
	void Seek(unsigned int pts, bool playAfterSeek);
private:
	void run();
	void timerEvent(QTimerEvent *event);

private:
	QMap<unsigned int, QList<YT_Frame_Ptr> > m_Frames;
	
	// Play/Pause
	volatile bool m_Paused;
};

#endif