#ifndef VIDEO_VIDEO_H
#define VIDEO_VIDEO_H

// Logical element of one video view
// handles zoom, panning and positioning of each video view

#include "YT_InterfaceImpl.h"
#include <QtCore>

class Renderer;
class Format;
class RendererWidget;
class QMouseEvent;
class SourceThread;
class QMainWindow;
class VideoView;
class ProcessThread;

class VideoQueue
{
};

struct TransformActionData 
{
	YTPlugIn* transformPlugin;
	QString transformName;
	QString outputName;
};

#define VV_SOURCE(vv)  ((vv)?vv->GetSource():NULL)
#define VV_VIDEOQUEUE(vv) ((vv)?vv->GetVideoQueue():NULL)
#define VV_LASTFRAME(vv) ((vv)?vv->GetLastFrame():FramePtr())

class VideoView : public QObject, public SourceCallback
{
	Q_OBJECT;

	friend class VideoViewList;
private:
	void Init(const char* path); // for source view
	void Init(Transform* transform, VideoQueue* source, QString outputName); // for tranform view
	void Init(unsigned int source, unsigned int processed); // for measure view
	void UnInit();
public:
	VideoView(QMainWindow* _mainWin, unsigned int viewId, RendererWidget* _parent, ProcessThread* processThread, PlaybackControl* control);
	
	virtual ~VideoView();
	QRect srcRect, dstRect;

	void SetZoomLevel(int mode);
	void SetGeometry(int x, int y, int width, int height);

	QAction* GetCloseAction() {return m_CloseAction; }

	PLUGIN_TYPE GetType() {return m_Type;}
	Source* GetSource();
	SourceInfo* GetSourceInfo() {return (m_Type == PLUGIN_SOURCE)?&m_SourceInfo:NULL;}
	SourceThread* GetSourceThread() {return m_SourceThread;} 
	Transform* GetTransform() {return m_Transform; }
	VideoQueue* GetVideoQueue() {return NULL;}
	QString GetOutputName() {return m_OutputName; }
	QDockWidget* GetDocketWidget() {return m_Dock;}
	unsigned int GetID() {return m_ViewID;}

	FramePtr GetLastFrame() {return m_LastFrame;}
	void SetLastFrame(FramePtr f) {m_LastFrame = f;}
	void ClearLastFrame() {m_LastFrame.clear();}
	
	void GetVideoSize( QSize& actual, QSize& display);

	// Update view point to ensure that this region is 
	void UpdateViewPort(double x, double y);
	
	// 
	void OnMouseMoveEvent( const QPoint& pt);
	void OnMousePressEvent( const QPoint& pt );
	void OnMouseReleaseEvent( const QPoint& pt);

	void ShowGui(Source*, bool show);
	void ResolutionDurationChanged();

	void UpdateMenu();
	QMenu* GetMenu();
signals:
	void ViewPortUpdated(VideoView*, double x, double y);
	void Close(VideoView*);
	void TransformTriggered(QAction*, VideoView*, TransformActionData*);

	void NeedVideoFormatReset();
public slots:
	void close();
	int width();
	int height();
	void setTitle(QString title);
	QString title();
	void setTimeStamps(QList<unsigned int>);

	void OnTransformTriggered();
	void OnDockFloating(bool);
protected:
	void RenderPlane(FramePtr, int plane);
	void RepositionVideo(bool emitSignal=false);
	void UpdateTransformActionList();
public:
	void computeAR( int src_width, int src_height, int& win_width, int& win_height );
private:
	PLUGIN_TYPE m_Type;
	SourceThread* m_SourceThread;
	ProcessThread* m_ProcessThread;
	PlaybackControl* m_Control;
	Transform* m_Transform; // Used for transform view
	QString m_OutputName;
	Measure* m_Measure; // Used for measure view
	// Render_Frame* m_RenderFrame;
	QMainWindow* m_MainWindow;

	SourceInfo m_SourceInfo;
	QString m_Title;

	QMenu* m_Menu;
	QAction* m_CloseAction;
	
	unsigned int m_ViewID;
	FramePtr m_LastFrame;

	int m_ScaleNum, m_ScaleDen;
	int m_SrcLeft, m_SrcTop, m_SrcWidth, m_SrcHeight;
	
	QDockWidget *m_Dock;
	QWidget* m_PluginGUI;

	QRect m_VideoViewRect;
	QPoint m_LastMousePoint;
	RendererWidget* parent;
};

#endif
