#ifndef CLICKABLE_SLIDER
#define CLICKABLE_SLIDER

#include <QtGui>

class QClickableSlider : public QSlider 
{
	Q_OBJECT;
public:
	QClickableSlider(QWidget* parent) {;}

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
};

#endif