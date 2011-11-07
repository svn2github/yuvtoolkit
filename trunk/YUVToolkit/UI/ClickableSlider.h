#ifndef CLICKABLE_SLIDER
#define CLICKABLE_SLIDER

#include <QtGui>

class QClickableSlider : public QSlider 
{
	Q_OBJECT;

	int selectionFrom, selectionTo;
public:
	QClickableSlider(QWidget* parent) : QSlider(Qt::Horizontal, parent), selectionFrom(0), selectionTo(-1) {;}

	void SetSelectionFrom(int i)
	{
		selectionFrom = i;
		update();
	}

	int GetSelectionFrom()
	{
		return selectionFrom;
	}

	void SetSelectionTo(int i)
	{
		selectionTo = i;
		update();
	}

	int GetSelectionTo()
	{
		return selectionTo;
	}
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

	void paintEvent(QPaintEvent *ev)
	{
		if (selectionTo>selectionFrom)
		{
			QPainter painter(this);
			painter.setBrush(Qt::darkGreen);

			QRect rcClient = this->rect();
			rcClient.setLeft(QStyle::sliderPositionFromValue(minimum(), maximum(), selectionFrom, width()));
			rcClient.setRight(QStyle::sliderPositionFromValue(minimum(), maximum(), selectionTo, width()));
			rcClient.setTop(rcClient.top()+rcClient.height()/2+1);
			rcClient.setBottom(rcClient.top()+3);
			painter.drawRect(rcClient);
		}
		

		QSlider::paintEvent(ev);
	}
};

#endif