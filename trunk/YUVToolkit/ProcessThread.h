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

	void Start(UintList sourceViewIDs);
	void Stop();

	bool IsPlaying();
signals:
	// Signals that one scene is ready for render
	void sceneReady(FrameListPtr scene, unsigned int pts, bool seeking);
	
public slots:
	void ReceiveFrame(FramePtr frame);
	
	void Play(bool play);

private slots:
	void ProcessFrameQueue();
private:
	void run();
	
	FrameList FastSeekQueue(unsigned int pts);
	void CleanQueue();
private:
	QMap<unsigned int, FrameList > m_Frames;
	UintList m_SourceViewIDs;
	
	// Play/Pause
	volatile bool m_Play;
};

#endif