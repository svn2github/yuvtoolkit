#ifndef TEXT_LABEL_H
#define TEXT_LABEL_H

#include <QtGui>

class TextLabel : public QLabel
{
	Q_OBJECT;
	Q_PROPERTY(QString text READ text WRITE setText)
public:
	TextLabel(QWidget* parent = 0) : QLabel(parent)
	{
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	}

	void setText(const QString &text)
	{
		content = text;
		update();
	}
	const QString & text() const { return content; }
protected:
	void paintEvent(QPaintEvent *event)
	{
		QRect rc = rect();
		rc.adjust(3,0,-3,0);

		QLabel::paintEvent(event);

		QPainter painter(this);
		QFontMetrics fontMetrics = painter.fontMetrics();
		QString elidedLastLine = fontMetrics.elidedText(content, Qt::ElideRight, rc.width());
		
		painter.drawText(rc, Qt::AlignLeft|Qt::AlignVCenter, elidedLastLine);
	}
private:
	QString content;
};

#endif