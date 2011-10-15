#ifndef YT_GLRENDERER_H
#define YT_GLRENDERER_H

#include "../YT_Interface.h"
#include <QtGui>
#include <QtOpenGL>

class YT_OpenGLRendererPlugin : public QObject, public YT_PlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YT_PlugIn);
public:
	virtual YT_RESULT Init(YT_Host*);

	virtual YT_Renderer* NewRenderer(QWidget* widget, const QString& name);
	virtual void ReleaseRenderer(YT_Renderer*);
};

class YT_OpenGLRenderer : public QGLWidget, public YT_Renderer
{
	Q_OBJECT;
public:
	YT_OpenGLRenderer(YT_Host* host, QWidget* widget, const QString& name);
	~YT_OpenGLRenderer();

	virtual QWidget* GetWidget() {return this;}

	virtual YT_RESULT RenderScene(QList<YT_Frame_Ptr> frames);

	// Allocate render specific buffers
	virtual YT_RESULT Allocate(YT_Frame_Ptr& frame, YT_Format_Ptr sourceFormat);
	virtual YT_RESULT Deallocate(YT_Frame_Ptr frame);

	// Prepare YT_Frame_Ptr before using it
	virtual YT_RESULT GetFrame(YT_Frame_Ptr& frame);
	virtual YT_RESULT ReleaseFrame(YT_Frame_Ptr frame);

	virtual bool NeedReset() {return false;}

	// If any function returns YT_E_RENDER_RESET, caller need to
	// 1. Release all render frames using ReleaseFrame(..)
	// 2. Deallocate all render frames using Deallocate(..)
	// 3. Call Reset()
	// 4. Reallocate frames again using Allocae(..)
	virtual YT_RESULT Reset() {return YT_OK;}

public slots:
	void OnResizeTimer();

protected:
	void paintEvent(QPaintEvent*);
	void showEvent(QShowEvent*);
	void resizeEvent(QResizeEvent*);

	QMutex m_MutexFramesRendered;
	QWaitCondition m_FramesRendered;

protected:
	YT_Host* m_Host;
	bool m_ReadyToRender;
	
	int m_BufferWidth;
	int m_BufferHeight;
	bool m_BufferSizeChanged;
};

#endif // YT_GLRENDERER_H
