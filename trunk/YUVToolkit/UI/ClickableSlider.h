#ifndef CLICKABLE_SLIDER
#define CLICKABLE_SLIDER

#include <QtGui>

class QClickableSlider : public QSlider 
{
	Q_OBJECT;
public:
	QClickableSlider(QWidget* parent) : QSlider(Qt::Horizontal, parent) {;}

protected:
	void mousePressEvent ( QMouseEvent * event )
	{
		if (event->button() == Qt::LeftButton) 
		{
			if (orientation() == Qt::Vertical)
			{
				setValue(QStyle::sliderValueFromPosition(minimum(),maximum(),event->x(),height(),0));
			}else
			{
				setValue(QStyle::sliderValueFromPosition(minimum(),maximum(),event->x(),width(),0));
			}

			event->accept();
		}

		QSlider::mousePressEvent(event);
	}

	/*
	void paintEvent(QPaintEvent *ev)
	{
		QPainter painter(this);
		painter.setBrush(Qt::darkGreen);
		
		QRect rcClient = this->rect();
		rcClient.setLeft(rcClient.left()+50);
		rcClient.setRight(rcClient.right()-50);
		rcClient.setTop(rcClient.top()+rcClient.height()/2+1);
		rcClient.setBottom(rcClient.top()+3);
		painter.drawRect(rcClient);

		QSlider::paintEvent(ev);
	}*/
};

#endif