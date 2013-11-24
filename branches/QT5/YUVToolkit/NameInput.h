#ifndef NameInput_H
#define NameInput_H

#include <QtGui>
#include "ui_NameInput.h"

class NameInput : public QDialog
{
	Q_OBJECT;
public:
	NameInput();
	~NameInput();
	QString getUserName();

public slots:
	//void changeUserName();
	void accept();
signals:

private:
	Ui::NameInput ui;
	QString username;
};

#endif // ScoreWindow_H
