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

	// Manage measure requests
	void SetMeasureRequests(const QList<MeasureRequest>& requests);
	// Manage measure results
	void GetMeasureResults(QList<MeasureResult>& results);
signals:
	// Signals that one scene is ready for render
	void sceneReady(FrameListPtr scene, unsigned int pts, bool seeking);
	
public slots:
	void ReceiveFrame(FramePtr frame);	

private slots:
	void ProcessFrameQueue();
private:
	void run();
	
	unsigned int GetFirstPTS(UintList sourceViewIds);
	unsigned int GetNextPTS(UintList sourceViewIds, unsigned int currentPTS);
	FrameListPtr FastSeekQueue(unsigned int pts, UintList sourceViewIds, bool& completed);
	bool CleanAndCheckQueue(UintList& sourceViewIds);
private:
	QMap<unsigned int, FrameList > m_SourceFrames;
	PlaybackControl* m_Control;
	unsigned int m_LastPTS;
	
	// List of source, transform and measure views
	QMutex m_MutexSource;
	UintList m_SourceViewIds;

	QMutex m_MutexMeasure;
	QList<MeasureRequest> m_MeasureRequests;
};

#endif