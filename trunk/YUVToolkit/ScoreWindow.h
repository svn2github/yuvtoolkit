#ifndef ScoreWindow_H
#define ScoreWindow_H

#include <QtGui>
#include <QStringList>
#include <QList>
#include "ui_ScoreWindow.h"
#include <algorithm>    // std::random_shuffle
#include <QScriptEngine>
#include <QScriptValue>
#include <QVariant>
#include <cstdlib>
#include <ctime>


class ScoreWindow : public QWidget
{
	Q_OBJECT;
public:
	ScoreWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ScoreWindow();

public slots:
	void enableButtons(bool, bool, bool);
	void createButton(int num, QString name);
	void createSlider(int num,int min, int max, QString name, QString scale);
	void updateCurSelect();
	void updateCurSlider();
	void initButton();
	void initSlider();
	QString getCurButtonResults(const QStringList& fileList);
	QString getCurSliderResults(const QStringList& fileList);
	void openResultsFile(QString fname, int mode);
	void writeResultsFile(QString results);
	void closeResultsFile();
	void changeButtonName(QString a, QString b, QString c);
#ifdef SHUFFLE  
	void regMetaType();
#endif
	QVariant shuffleList(QVariant origin, bool shuffle_scene, bool shuffle_vide, bool keep_ref);

signals:
	void onNext();
	void onPrevious();
	void onFinish();
private:
	Ui::ScoreWindow ui;
	QStringList ButtonNameList;
	QStringList SliderNameList;
	QStringList ScaleNameList;
	QRadioButton** ButtonList;
	QLabel * label_cur_selected;
	QLabel ** label_cur_selected_list;
	QSlider** SliderList;
	int NumOfConditions;
	QFile * FileHandle;
	QScriptEngine engine;
};

#endif // ScoreWindow_H
