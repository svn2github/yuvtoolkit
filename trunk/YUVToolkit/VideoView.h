#ifndef VIDEO_VIDEO_H
#define VIDEO_VIDEO_H

// Logical element of one video view
// handles zoom, panning and positioning of each video view

#include "YT_InterfaceImpl.h"
#include <QtCore>

class YT_Renderer;
class YT_Format;
class RendererWidget;
class QMouseEvent;
class VideoQueue;
class SourceThread;
class QMainWindow;
class VideoView;

struct TransformActionData 
{
	YT_PlugIn* transformPlugin;
	QString transformName;
	QString outputName;
};

#define VV_SOURCE(vv)  ((vv)?vv->GetSource():NULL)
#define VV_VIDEOQUEUE(vv) ((vv)?vv->GetVideoQueue():NULL)
#define _VV_LASTFRAME(vv) (VV_VIDEOQUEUE(vv)?VV_VIDEOQUEUE(vv)->GetLastRenderFrame():NULL)
#define VV_LASTFRAME(vv) (_VV_LASTFRAME(vv)?_VV_LASTFRAME(vv)->source:NULL)

class VideoView : public QObject, public sigslot::has_slots<>
{
	Q_OBJECT;

	YT_PLUGIN_TYPE m_Type;
	VideoQueue* m_VideoQueue;
	SourceThread* m_SourceThread;
	VideoQueue* m_RefVideoQueue; // Used for transform or measure view
	YT_Transform* m_Transform; // Used for transform view
	QString m_OutputName;
	YT_Measure* m_Measure; // Used for measure view
	// YT_Render_Frame* m_RenderFrame;
	QMainWindow* m_MainWindow;
	
	int m_VideoWidth;
	int m_VideoHeight;
	int m_Duration;
	QString m_Title;

	QAction* m_CloseAction;
	bool m_TransformActionListUpdated;
	QList<QAction*> m_TransformActionList;
	void UpdateTransformActionList();
public:
	VideoView(QMainWindow* _mainWin, RendererWidget* _parent);
	void Init(const char* path, unsigned int pts); // for source view
	void Init(YT_Transform* transform, VideoQueue* source, QString outputName); // for tranform view
	void Init(YT_Measure* measure, VideoQueue* source, VideoQueue* source1); // for measure view
	void UnInit();
	virtual ~VideoView();
	QRect srcRect, dstRect;

	void SetZoomLevel(int mode);
	void SetGeometry(int x, int y, int width, int height);

	void SetTitle(const char* title);
	const QString& GetTitle() {return m_Title; }
	const QList<QAction*>& GetTransformActions() {return m_TransformActionList; }
	QAction* GetCloseAction() {return m_CloseAction; }

	YT_PLUGIN_TYPE GetType() {return m_Type;}
	YT_Source* GetSource();
	SourceThread* GetSourceThread() {return m_SourceThread;} 
	YT_Transform* GetTransform() {return m_Transform; }
	VideoQueue* GetVideoQueue() {return m_VideoQueue;}
	QString GetOutputName() {return m_OutputName; }
	QDockWidget* GetDocketWidget() {return m_Dock;}
	
	VideoQueue* GetRefVideoQueue() {return m_RefVideoQueue;}	

	void GetVideoSize( QSize& actual, QSize& display);

	// Update view point to ensure that this region is 
	void UpdateViewPort(double x, double y);
	bool CheckResolutionDurationChanged();

	// 
	void OnMouseMoveEvent( const QPoint& pt);
	void OnMousePressEvent( const QPoint& pt );
	void OnMouseReleaseEvent( const QPoint& pt);

	void OnSourceGUINeeded();
signals:
	void ViewPortUpdated(VideoView*, double x, double y);
	void Close(VideoView*);
	void TransformTriggered(QAction*, VideoView*, TransformActionData*);
public slots:
	void OnClose();
	void OnTransformTriggered();

	void OnDockFloating(bool);
protected:
	void RenderPlane(YT_Frame_Ptr, int plane);
	void RepositionVideo(bool emitSignal=false);
	void computeAR( int src_width, int src_height, int& win_width, int& win_height );
private:
	int m_ScaleNum, m_ScaleDen;
	int m_SrcLeft, m_SrcTop, m_SrcWidth, m_SrcHeight;
	
	QDockWidget *m_Dock;
	QWidget* m_PluginGUI;

	QRect m_VideoViewRect;
	QPoint m_LastMousePoint;
	RendererWidget* parent;

	YT_Format_Ptr m_RenderFormat;
	YT_Format_Ptr m_SourceFormat;
	YT_Frame_Ptr m_EmptyFrame;
};
#endif