#ifndef RAWFORMATWIDGET_H
#define RAWFORMATWIDGET_H

#include "../YT_Interface.h"

#include <QtGui>
#include <QtCore>

#include "ui_RawFormatWidget.h"

class YTS_Raw;
class RawFormatWidget : public QWidget
{
	Q_OBJECT

public:
	RawFormatWidget(YTS_Raw*, QWidget *parent = 0);
	~RawFormatWidget();
protected:
	void showEvent(QShowEvent *event);
public slots:
	void OnApply();
private:
	Ui::RawFormatWidget ui;
	YTS_Raw* rawSource;
	YT_Format_Ptr m_Format;
};

#endif // RAWFORMATWIDGET_H
