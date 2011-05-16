#ifndef VIDEO_RENDER_WIDGET_H
#define VIDEO_RENDER_WIDGET_H

#include <QtCore>
#include <QtGui/QWidget>

class YT_Renderer;
class YT_PlugIn;
class Layout;

class RendererWidget : public QWidget
{
	Q_OBJECT;
public:
	RendererWidget(QWidget* parent);
	~RendererWidget(void);

	Layout* layout;

	void Init(const QString& renderType);
	void UnInit();

	YT_Renderer* GetRenderer() {return m_Renderer;}
	YT_PlugIn* GetRenderPlugIn() {return m_Plugin;}

signals:
	void repositioned();
protected:
	virtual void mouseMoveEvent( QMouseEvent* );
	virtual void mousePressEvent( QMouseEvent* );
	virtual void mouseReleaseEvent( QMouseEvent* );
	virtual void resizeEvent ( QResizeEvent* );
	virtual void moveEvent (QMoveEvent *);

	YT_Renderer* m_Renderer;
	void paintEvent(QPaintEvent*);

	QImage m_Background;
	YT_PlugIn* m_Plugin;
};

#endif