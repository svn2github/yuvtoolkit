#ifndef ScoreWindow_H
#define ScoreWindow_H

#include <QtGui>
#include "ui_ScoreWindow.h"

class ScoreWindow : public QWidget
{
	Q_OBJECT;
public:
	ScoreWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ScoreWindow();

public slots:
	void enableButtons(bool, bool, bool);
signals:
	void onNext();
	void onPrevious();
	void onFinish();
private:
	Ui::ScoreWindow ui;
};

#endif // ScoreWindow_H
