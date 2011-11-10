#include "YT_Interface.h"

#include "MeasureWindow.h"
#include "VideoView.h"
#include "VideoViewList.h"



MeasureResultsModel::MeasureResultsModel( QObject *parent, int nrColumns ) :
QAbstractTableModel(parent), m_Columns(nrColumns)
{
	MeasureResult res = {0};
	res.requestId = 0;
	res.measureName = "MSE";
	m_Results.append(res);

	res.requestId = 0;
	res.measureName = "PSNR";
	m_Results.append(res);
}

int MeasureResultsModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return 8;
}

int MeasureResultsModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return m_Columns;
}

QVariant MeasureResultsModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const
{
	int row = index.row();
	int col = index.column();

	switch(role){
	case Qt::DisplayRole:
		if (row == 0 && col == 1) return QString("<--left");
		if (row == 1 && col == 1) return QString("right-->");

		return QString("%1, %2")
			.arg(m_Results.at(0).results[0])
			.arg(col +1);
		break;
	case Qt::FontRole:
		if (row == 0 && col == 0) //change font only for cell(0,0)
		{
			QFont boldFont;
			boldFont.setBold(true);
			return boldFont;
		}
		break;
	case Qt::BackgroundRole:
		if (row == 1 && col == 1)  //change background only for cell(1,2)
		{
			QBrush redBackground(Qt::lightGray);
			return redBackground;
		}
		break;
	case Qt::CheckStateRole:

		if (row == 1 && col == 0) //add a checkbox to cell(1,0)
		{
			return Qt::Checked;
		}
	}
	return QVariant();
}

QVariant MeasureResultsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
			case 0:
				return QString("first");
			case 1:
				return QString("second");
			case 2:
				return QString("third");
			}
		}else {
			switch (section)
			{
			case 0:
				return QString("PSNR-Y");
			case 1:
				return QString("PSNR-U");
			case 2:
				return QString("PSNR-V");
			case 3:
				return QString("PSNR");
			case 4:
				return QString("MSE-Y");
			case 5:
				return QString("MSE-U");
			case 6:
				return QString("MSE-V");
			case 7:
				return QString("MSE");
			}
		}
	}
	return QVariant();

}

Qt::ItemFlags MeasureResultsModel::flags( const QModelIndex & /*index*/ ) const
{
	return Qt::ItemIsSelectable |  Qt::ItemIsEnabled ;

}

void MeasureResultsModel::ResultsUpdated()
{
	emit dataChanged(index(0,0), index(8, 1));
}

DistortionMapModel::DistortionMapModel( QObject *parent ) :
QAbstractTableModel(parent)
{

}

int DistortionMapModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return 2;
}

int DistortionMapModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return 1;
}

QVariant DistortionMapModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const
{
	int row = index.row();
	int col = index.column();

	switch(role){
	case Qt::DisplayRole:
		switch (index.row())
		{
		case 0:
			return QString("PSNR map");
		case 1:
			return QString("MSE map");
		}
		break;
	case Qt::CheckStateRole:
		return Qt::Checked;
	}
	return QVariant();
}

Qt::ItemFlags DistortionMapModel::flags( const QModelIndex & /*index*/ ) const
{
	return Qt::ItemIsSelectable |  Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}


MeasureWindow::MeasureWindow(VideoViewList* vvList, QWidget *parent, Qt::WFlags flags) : 
	QWidget(parent, flags), m_VideoViewList(vvList), m_ResultsModel(NULL), m_DistortionMapModel(this), m_UpdateTimer(NULL)
{
	ui.setupUi(this);
	// connect(ui.originalList, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboIndexChanged(int)));
	// connect(ui.processedList, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboIndexChanged(int)));

	ui.table_Distortion_Map->setModel(&m_DistortionMapModel);
	ui.table_Distortion_Map->setColumnWidth(0, 1000);
}

MeasureWindow::~MeasureWindow()
{
	ClearTimer();
	SAFE_DELETE(m_ResultsModel);
}

void MeasureWindow::showEvent( QShowEvent *event )
{
	UpdateRequest();
}

void MeasureWindow::hideEvent( QHideEvent *event )
{
	ClearTimer();

	QList<MeasureRequest> list;
	m_VideoViewList->GetProcessThread()->SetMeasureRequests(list);
}


void MeasureWindow::onComboIndexChanged( int )
{
	/*if (ui.originalList->count()<=1)
	{
		return;
	}

	m_Original = m_VideoViewList->at(ui.originalList->currentIndex());
	m_Processed = m_VideoViewList->at(ui.processedList->currentIndex());

	if (m_MeasureList.size()>0)
	{
		for (int i=0; i<m_MeasureList.size(); i++)
		{
			// TODO clean up.
		}
	}

	const QList<PlugInInfo*>& lst = GetHostImpl()->GetMeasurePluginList();
	for (int i=0; i<lst.size(); i++)
	{
		PlugInInfo* plugInInfo = lst[i];

		Measure* measure = plugInInfo->plugin->NewMeasure(plugInInfo->string);
		m_MeasureList.append(measure);

		SourceInfo origInfo, procInfo;
		m_Original->GetSource()->GetInfo(origInfo);
		m_Processed->GetSource()->GetInfo(procInfo);
		measure->GetSupportedModes(origInfo.format, procInfo.format, m_ViewOutItems, m_MeasureOutItems);

		for (int i=0; i<m_MeasureOutItems.size(); i++)
		{
			Measure::MeasureItem item = m_MeasureOutItems.at(i);
			m_OutputMeasureItems.insert(item, QVariant(-1));
		}
	}

	*/
}

void MeasureWindow::UpdateMeasure()
{
	/*
	if (ui.originalList->count()<=1)
	{
		return;
	}

	VideoQueue::Frame* origFrame  = m_Original->GetVideoQueue()->GetLastRenderFrame();			
	VideoQueue::Frame* procFrame  = m_Processed->GetVideoQueue()->GetLastRenderFrame();			
	if (!(origFrame && procFrame))
	{
		return;
	}

	Measure* measure = m_MeasureList.at(0);

	measure->Process(origFrame->source, procFrame->source, m_OutputViewItems, m_OutputMeasureItems);*/
}

void MeasureWindow::UpdateMeasureWindow()
{
	if (!isVisible())	
	{
		// TODO; enable timer only when needed
		return;
	}

	/*QMapIterator<Measure::MeasureItem, QVariant> i(m_OutputMeasureItems);
	while (i.hasNext()) {
		i.next();
		// cout << i.key() << ": " << i.value() << endl;
		QTableWidgetItem* item = ui.tableWidget->item(i.key().plane, i.key().measureType);
		if (item)
		{
			item->setText(i.value().toString());
		}
	}
	*/
}

QSize MeasureWindow::sizeHint() const
{
	QTableView* view = ui.table_Measure_Results;
	QAbstractItemModel* m = view->model();

	int colCount = m->columnCount();
	int w = 0;
	for(int i=0; i<colCount; ++i) {
		w += view->columnWidth(i);
	}
	w += view->verticalHeader()->width();

	w += qMax<int>(width()-view->width(), 0);
	w += 10;
	return QSize(w, 250);
}

void MeasureWindow::on_button_Options_clicked()
{
	
}

void MeasureWindow::ClearTimer()
{
	if (m_UpdateTimer)
	{
		m_UpdateTimer ->stop();
		SAFE_DELETE(m_UpdateTimer);
	}
}

void MeasureWindow::OnTimer()
{
	if (m_ResultsModel)
	{
		m_VideoViewList->GetProcessThread()->GetMeasureResults(m_ResultsModel->GetResults());
		m_ResultsModel->ResultsUpdated();
	}
}

void MeasureWindow::UpdateRequest()
{
	QList<MeasureRequest> list;
	MeasureRequest req = {0};
	req.measureName = "MSE";
	req.requestId = 0;
	req.showDistortionMap = false;
	req.sourceViewId1 = 0;
	req.sourceViewId2 = 1;
	list.append(req);
	m_VideoViewList->GetProcessThread()->SetMeasureRequests(list);

	ClearTimer();


	m_UpdateTimer = new QTimer(this);
	m_UpdateTimer->setInterval(200);
	connect(m_UpdateTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));
	m_UpdateTimer->start();
}

void MeasureWindow::OnVideoViewSourceListChanged()
{
	UintList sourceList = m_VideoViewList->GetSourceIDList();
	TextLabel* labels[] = {ui.label_Original, ui.label_Processed, ui.label_Processed_2};
	QLabel* labels2[] = {ui.label_0, ui.label_1, ui.label_2};

	for (int i=0; i<sizeof(labels)/sizeof(labels[0]); i++)
	{
		if (sourceList.size()>i)
		{
			VideoView* vv = m_VideoViewList->find(sourceList.at(i));
			labels[i]->setText(vv->GetTitle());
			labels[i]->setToolTip(vv->GetTitle());
			labels[i]->show();
			labels2[i]->show();
		}else
		{
			labels[i]->setText("");
			labels[i]->setToolTip("");
			labels[i]->hide();
			labels2[i]->hide();
		}
	}

	SAFE_DELETE(m_ResultsModel);
	m_ResultsModel = new MeasureResultsModel(this, ((sourceList.size()>2)?2:1));

	ui.table_Measure_Results->setModel(m_ResultsModel);
	ui.table_Measure_Results->resizeColumnsToContents();
}
