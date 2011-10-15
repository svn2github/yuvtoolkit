#ifndef PROCESS_THREAD_H
#define PROCESS_THREAD_H

#include "YT_InterfaceImpl.h"
#include <QThread>
#include <QList>

class ProcessThread : public QThread
{
	Q_OBJECT;
public:
	ProcessThread();
	~ProcessThread();

	void Start();
	void Stop();

signals:
	void sceneReady(QList<YT_Frame_Ptr> scene, unsigned int renderPTS);
	
public slots:
	void ReceiveFrame(YT_Frame_Ptr frame);
	
private:
	void run();
	void timerEvent(QTimerEvent *event);

private:
	QList<YT_Frame_Ptr> m_Frames;
};

#endif