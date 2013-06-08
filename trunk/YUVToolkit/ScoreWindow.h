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
#include <QWheelEvent>

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
	void createSliderandButton(int num,int min, int max, QString name, QString scale);
	void updateCurSelect();
	void updateCurSlider();
	void initButton();
	void initSlider(bool slider_status = false);
	void enableSlider(int i);
	QString getCurButtonResults(const QStringList& fileList);
	QString getCurSliderResults(const QStringList& fileList);
	void getCurSliderResultsPlusTimestamp(QString timestamp);
	void openResultsFile(QString fname, int mode);
	void writeResultsFile(QString results);
	void closeResultsFile();
	void writeSSCQEResultsFile();
	void changeButtonName(QString a, QString b, QString c);
	void playVideo(int i);
	void SetVideoListInCurrentScene(QStringList cur);
	QVariant shuffleList(QVariant origin, bool shuffle_scene, bool shuffle_vide, bool keep_ref);
	void setCurrentVideo(QString cur);
	void enableRestrictMouse(bool status = true);

signals:
	void onNext();
	void onPrevious();
	void onFinish();
	void playVideoInMainWindow(QString filename);

protected:
	void wheelEvent(QWheelEvent *event);
	void mouseMoveEvent(QMouseEvent * event);

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
	QStringList VideoListInCurrentScene;
	QString currentVideo;
	QString sscqeresult;
	bool restrictMouse;
};

#endif // ScoreWindow_H
