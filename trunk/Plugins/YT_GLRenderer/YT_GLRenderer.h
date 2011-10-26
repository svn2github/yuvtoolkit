#ifndef GLRENDERER_H
#define GLRENDERER_H

#include "../YT_Interface.h"
#include <QtGui>
#include <QtOpenGL>

class OpenGLRendererPlugin : public QObject, public YTPlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YTPlugIn);
public:
	virtual RESULT Init(Host*);

	virtual Renderer* NewRenderer(QWidget* widget, const QString& name);
	virtual void ReleaseRenderer(Renderer*);
};

class OpenGLRenderer : public QGLWidget, public Renderer
{
	Q_OBJECT;
public:
	OpenGLRenderer(Host* host, QWidget* widget, const QString& name);
	~OpenGLRenderer();

	virtual QWidget* GetWidget() {return this;}

	virtual RESULT RenderScene(const FrameList& frames);

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

public slots:
	void OnResizeTimer();

protected:
	void paintEvent(QPaintEvent*);
	void showEvent(QShowEvent*);
	void resizeEvent(QResizeEvent*);

	QMutex m_MutexFramesRendered;
	QWaitCondition m_FramesRendered;

protected:
	Host* m_Host;
	bool m_ReadyToRender;
	
	int m_BufferWidth;
	int m_BufferHeight;
	bool m_BufferSizeChanged;
};

#endif // GLRENDERER_H
