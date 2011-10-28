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

	void Start();
	void Stop();

	// Manage list of source, transform and measure views
	void SetSources(UintList sourceViewIds);
signals:
	// Signals that one scene is ready for render
	void sceneReady(FrameListPtr scene, unsigned int pts, bool seeking);
	
public slots:
	void ReceiveFrame(FramePtr frame);	

private slots:
	void ProcessFrameQueue();
private:
	void run();
	
	FrameListPtr FastSeekQueue(unsigned int pts, UintList sourceViewIds);
	void CleanQueue(UintList& sourceViewIds);
private:
	QMap<unsigned int, FrameList > m_SourceFrames;
	PlaybackControl* m_Control;
	
	// List of source, transform and measure views
	QMutex m_Mutex;
	UintList m_SourceViewIds;
};

#endif