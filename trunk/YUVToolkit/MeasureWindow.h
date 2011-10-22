#ifndef MEASURE_WINDOW_H
#define MEASURE_WINDOW_H

#include "YT_Interface.h"
#include <QtGui>
#include "ui_MeasureWindow.h"
class VideoViewList;
class VideoView;

class MeasureWindow : public QWidget
{
	Q_OBJECT;
	VideoViewList* m_VideoViewList;

	VideoView* m_Original;
	VideoView* m_Processed;
	QList<Measure*> m_MeasureList;
	QList<Measure::MeasureItem> m_MeasureOutItems;
	QList<Measure::MeasureItem> m_ViewOutItems;

	QMap<Measure::MeasureItem, FramePtr> m_OutputViewItems;
	QMap<Measure::MeasureItem, QVariant> m_OutputMeasureItems;
public:
	Ui::MeasureWindow ui;

	MeasureWindow(VideoViewList* vvList, QWidget *parent = 0, Qt::WFlags flags = 0);
	~MeasureWindow();

	void UpdateMeasure();

	void UpdateMeasureWindow();
protected:
	void showEvent(QShowEvent *event);

public slots:
	void onComboIndexChanged(int);
};

#endif
