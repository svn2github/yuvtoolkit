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
	ProcessThread(PlaybackControl*);
	~ProcessThread();

	void Start(UintList sourceViewIDs);
	void Stop();
signals:
	// Signals that one scene is ready for render
	void sceneReady(FrameListPtr scene, unsigned int pts, bool seeking);
	
public slots:
	void ReceiveFrame(FramePtr frame);	

private slots:
	void ProcessFrameQueue();
private:
	void run();
	
	FrameListPtr FastSeekQueue(unsigned int pts);
	void CleanQueue();
private:
	QMap<unsigned int, FrameList > m_Frames;
	UintList m_SourceViewIDs;
	PlaybackControl* m_Control;
	PlaybackControl::Status m_Status;
};

#endif