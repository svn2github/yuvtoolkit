#include "YT_Interface.h"
#include "RendererWidget.h"
#include "SourceThread.h"
#include "YT_InterfaceImpl.h"
#include "Layout.h"

#include <assert.h>

void RendererWidget::paintEvent( QPaintEvent* )
{
	if (m_Renderer)
	{
	}else
	{
		QRect rcClient = this->rect();

		QPainter painter(this);
		painter.setBrush(Qt::black);
		painter.drawRect(rcClient);

		QPoint pt(0,0);
		pt.setX((rcClient.width()-m_Background.width())/2);
		pt.setY((rcClient.height()-m_Background.height())/2);

		painter.drawImage(pt, m_Background);
/*
		if (m_RenderWindow)
		{
			painter.setPen(grey);

			QPoint pos = m_RenderWindow->pos();
			QSize size = m_RenderWindow->size();
			painter.drawRect(pos.x()-1, pos.y()-1, size.width()+1, size.height()+1);
		}*/
	}
	
}

RendererWidget::RendererWidget( QWidget* parent ) : 
	QWidget(parent), m_Renderer(NULL), m_Plugin(NULL)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true); // don't clear the area before the paintEvent
	setAttribute(Qt::WA_PaintOnScreen, true); // disable double buffering

	m_Background.load(QString::fromUtf8(":/RawVideoToolkit/Resources/background.png"), "PNG");

	// setMinimumSize(160, 120);

	layout = new Layout(this);
}

RendererWidget::~RendererWidget( void )
{
	SAFE_DELETE(layout);
}

void RendererWidget::Init(const QString& renderType)
{
	assert(m_Renderer==0);

	m_Plugin = GetHostImpl()->FindRenderPlugin(renderType);	
	m_Renderer = m_Plugin->NewRenderer(this, renderType);

	QWidget* widget = m_Renderer->GetWidget();
	if (widget)
	{
		widget->show();
		widget->resize(size());
	}
}

void RendererWidget::UnInit()
{
	m_Plugin->ReleaseRenderer(m_Renderer);
	m_Renderer = 0;
}

void RendererWidget::mouseMoveEvent( QMouseEvent* e )
{
	layout->OnMouseMoveEvent(e);
}

void RendererWidget::mousePressEvent( QMouseEvent* e )
{
	layout->OnMousePressEvent(e);
}

void RendererWidget::mouseReleaseEvent( QMouseEvent* e )
{
	layout->OnMouseReleaseEvent(e);
}

void RendererWidget::resizeEvent( QResizeEvent* e)
{
	if (m_Renderer)
	{
		QWidget* widget = m_Renderer->GetWidget();
		if (widget)
		{
			widget->resize(e->size());
		}
	}

	emit repositioned();
}

void RendererWidget::moveEvent( QMoveEvent * )
{
	emit repositioned();
}