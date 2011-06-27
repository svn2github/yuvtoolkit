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
	QList<YT_Measure*> m_MeasureList;
	QList<YT_Measure::YT_Measure_Item> m_MeasureOutItems;
	QList<YT_Measure::YT_Measure_Item> m_ViewOutItems;

	QMap<YT_Measure::YT_Measure_Item, YT_Frame_Ptr> m_OutputViewItems;
	QMap<YT_Measure::YT_Measure_Item, QVariant> m_OutputMeasureItems;
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
