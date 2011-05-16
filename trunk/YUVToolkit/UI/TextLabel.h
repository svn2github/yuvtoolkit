#ifndef TEXT_LABEL_H
#define TEXT_LABEL_H

#include <QtGui>

class TextLabel : public QTextEdit
{
	Q_OBJECT;
public:
	TextLabel(const QString& text, QWidget* parent = 0) : QTextEdit(text, parent)
	{
		setReadOnly(true);
		setFrameStyle(QFrame::NoFrame);
		QPalette pal = palette();
		pal.setColor(QPalette::Base, Qt::transparent);
		setPalette(pal);

		setLineWrapMode(QTextEdit::WidgetWidth);
		setWordWrapMode(QTextOption::WrapAnywhere);
		//label->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
		connect(document()->documentLayout(), 
			SIGNAL(documentSizeChanged(QSizeF)), 
			this, SLOT(OnAdjustMinimumSize(QSizeF)));
	}


private slots:
	void OnAdjustMinimumSize(const QSizeF& size)
	{
		//setMinimumHeight(size.height() + 2 * frameWidth());
		setMinimumHeight(size.height());
		setMaximumHeight(size.height());
	}
};

#endif