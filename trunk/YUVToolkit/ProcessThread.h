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

signals:
	void sceneReady(QList<YT_Frame_Ptr> scene, unsigned int renderPTS);
	
public slots:
	void ReceiveFrame(YT_Frame_Ptr frame);
	void SetLayout(QList<unsigned int>, QList<QRect>, QList<QRect>);
	
private:
	void run();
	void timerEvent(QTimerEvent *event);

private:
	QMap<unsigned int, QList<YT_Frame_Ptr> > m_Frames;
	// Layout
	QList<unsigned int> m_ViewIDs;
	QList<QRect> m_SrcRects;
	QList<QRect> m_DstRects;
};

#endif