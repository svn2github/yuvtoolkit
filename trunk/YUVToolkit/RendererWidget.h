#ifndef VIDEO_RENDER_WIDGET_H
#define VIDEO_RENDER_WIDGET_H

#include <QtCore>
#include <QtGui/QWidget>

class Renderer;
class YTPlugIn;
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

	Renderer* GetRenderer() {return m_Renderer;}
	YTPlugIn* GetRenderPlugIn() {return m_Plugin;}

signals:
	void repositioned();
protected:
	virtual void mouseMoveEvent( QMouseEvent* );
	virtual void mousePressEvent( QMouseEvent* );
	virtual void mouseReleaseEvent( QMouseEvent* );
	virtual void resizeEvent ( QResizeEvent* );
	virtual void moveEvent (QMoveEvent *);

	Renderer* m_Renderer;
	void paintEvent(QPaintEvent*);

	QImage m_Background;
	YTPlugIn* m_Plugin;
};

#endif