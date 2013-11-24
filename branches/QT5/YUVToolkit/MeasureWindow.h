#ifndef MEASURE_WINDOW_H
#define MEASURE_WINDOW_H

#include "YT_InterfaceImpl.h"
#include <QTableView>
#include <QWidget>
#include <QtCore>
class VideoViewList;
class VideoView;

#include <QAbstractTableModel>

class MeasureResultsModel : public QAbstractTableModel
{
	Q_OBJECT;
	QList<MeasureItem>& m_Results;
	QStringList m_MeasureNameRows;
	QList<unsigned int> m_SourceView1Ids;
	QList<unsigned int> m_SourceView2Ids;
	
public:
	MeasureResultsModel(QObject *parent, QList<MeasureItem>& );
	int rowCount(const QModelIndex &parent = QModelIndex()) const ;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	Qt::ItemFlags flags(const QModelIndex & /*index*/) const;
	void ResultsUpdated();
};

class MeasureWindow : public QMainWindow
{
	Q_OBJECT;
	VideoViewList* m_VideoViewList;


	QToolBar* m_ToolBar;
	QTableView* m_ResultsTable;
	MeasureResultsModel* m_ResultsModel;
	QTimer* m_UpdateTimer;

	UintList m_SourceList;
	QList<MeasureItem> m_MeasureItemList;
	bool m_ShowDisortionMap;
public:
    MeasureWindow(VideoViewList* vvList, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~MeasureWindow();

	QToolBar* GetToolBar() {return m_ToolBar;}
	
	QSize sizeHint() const;
protected:
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	
	void ClearAll();
	void UpdateRequest();
	void UpdateLabels();
public slots:
	void OnVideoViewSourceListChanged();
	void OnShowDistortionMap(bool);
	void OnOptionChanged();
private slots:
	void on_button_Options_clicked();
	void OnTimer();
};

#endif
