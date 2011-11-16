#ifndef OPTIONS_H
#define OPTIONS_H

#include <QtGui>
#include "ui_Options.h"

class Options : public QDialog
{
	Q_OBJECT;
public:
	Options(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Options();

private:
	Ui::Options ui;
};

#endif // OPTIONS_H
