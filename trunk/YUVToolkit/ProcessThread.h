#ifndef PROCESS_THREAD_H
#define PROCESS_THREAD_H

#include "YT_InterfaceImpl.h"
#include <QThread>
#include <QList>
#include <QMap>

class ProcessThread : public QThread, public PlaybackInfo
{
	Q_OBJECT;
public:
	ProcessThread();
	~ProcessThread();

	void Start(UintList sourceViewIDs);
	void Stop();

	virtual bool IsPlaying();
	virtual unsigned int LastPTS();
	virtual unsigned int SeekingPTS();
signals:
	// Signals that one scene is ready for render
	void sceneReady(FrameListPtr scene, unsigned int pts, bool seeking);
	
public slots:
	void ReceiveFrame(FramePtr frame);	
	void Play(bool play);
	void Seek(unsigned int pts);

private slots:
	void ProcessFrameQueue();
private:
	void run();
	
	FrameListPtr FastSeekQueue(unsigned int pts);
	void CleanQueue();
private:
	QMap<unsigned int, FrameList > m_Frames;
	UintList m_SourceViewIDs;
	
	// Play/Pause
	volatile bool m_Play;

	volatile unsigned int m_LastPTS;
	volatile unsigned int m_SeekingPTS;
};

#endif