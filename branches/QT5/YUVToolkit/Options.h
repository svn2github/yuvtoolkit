#ifndef OPTIONS_H
#define OPTIONS_H

#include <QWidget>
#include "ui_Options.h"

class Options : public QDialog
{
	Q_OBJECT;
public:
    Options(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~Options();

	int exec(int index);
signals:
	void OptionChanged();
private slots:
	void OnAccepted();
	void on_button_File_Associations_clicked();
	
	void OnMeasureSelected(QListWidgetItem * current, QListWidgetItem * previous);
	void OnMeasureChanged(QListWidgetItem * item);
private:
	void LoadMeasureOptions(QListWidgetItem * current);
	void SaveMeasureOptions(QListWidgetItem * current);
private:
	struct MeasureOptions
	{
		bool hasDistMap;
		bool showDistMap;
		float upperRange;
		float lowerRange;
		QString info;
		// unsigned int overshootColor;
		// unsigned int undershootColor;
	};

	QMap<QString, MeasureOptions> m_MeasureOptions;
	Ui::Options ui;
};

#endif // OPTIONS_H
