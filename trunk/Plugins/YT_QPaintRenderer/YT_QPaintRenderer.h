#ifndef QPAINT_RENDERER_H
#define QPAINT_RENDERER_H

#include "../YT_Interface.h"
#include <QtGui>

class QPaintRendererPlugin : public QObject, public YTPlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YTPlugIn);
public:
	virtual RESULT Init(Host*);

	virtual Renderer* NewRenderer(QWidget* widget, const QString& name);
	virtual void ReleaseRenderer(Renderer*);
};

class QPaintRenderer : public QWidget, public Renderer
{
	Q_OBJECT;
public:
	QPaintRenderer(Host* host, QWidget* widget, const QString& name);
	~QPaintRenderer();

	virtual QWidget* GetWidget() {return this;}

	virtual RESULT RenderScene(FrameList frames);

	// Allocate render specific buffers
	virtual RESULT Allocate(FramePtr& frame, FormatPtr sourceFormat);
	virtual RESULT Deallocate(FramePtr frame);

	// Prepare FramePtr before using it
	virtual RESULT GetFrame(FramePtr& frame);
	virtual RESULT ReleaseFrame(FramePtr frame);

	virtual bool NeedReset() {return false;}

	// If any function returns E_RENDER_RESET, caller need to
	// 1. Release all render frames using ReleaseFrame(..)
	// 2. Deallocate all render frames using Deallocate(..)
	// 3. Call Reset()
	// 4. Reallocate frames again using Allocae(..)
	virtual RESULT Reset() {return OK;}

protected:
	void paintEvent(QPaintEvent*);

	QMutex m_MutexFramesRendered;
	QWaitCondition m_FramesRendered;

	FrameList m_Frames;
protected:
	Host* m_Host;
};

#endif // QPAINT_RENDERER_H
