#include "YT_Interface.h"

#include "MeasureWindow.h"
#include "VideoView.h"
#include "VideoViewList.h"
#include "Settings.h"

MeasureResultsModel::MeasureResultsModel( QObject *parent, QList<MeasureItem>& results) :
QAbstractTableModel(parent), m_Results(results)
{
	for (int i=0; i<m_Results.size(); i++)
	{
		const MeasureItem& item = m_Results.at(i);

		bool found = false;
		for (int j=0; j<m_SourceView1Ids.size(); j++)
		{
			if (m_SourceView1Ids.at(j) == item.sourceViewId1 && m_SourceView2Ids.at(j) == item.sourceViewId2)
			{
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			m_SourceView1Ids.append(item.sourceViewId1);
			m_SourceView2Ids.append(item.sourceViewId2);
		}

		if (item.sourceViewId1 == m_SourceView1Ids.at(0) && item.sourceViewId2 == m_SourceView2Ids.at(0))
		{
			m_MeasureNameRows.append(item.op.measureName);
		}
	}
}

int MeasureResultsModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return m_MeasureNameRows.size()*4;
}

int MeasureResultsModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return m_SourceView1Ids.size();
}

QVariant MeasureResultsModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const
{
	int row = index.row();
	int col = index.column();

	switch(role){
	case Qt::DisplayRole:
		{
			unsigned int sourceViewId1 = m_SourceView1Ids.at(col);
			unsigned int sourceViewId2 = m_SourceView2Ids.at(col);
			const QString& measureName = m_MeasureNameRows.at(row/4);

			for (int i=0; i<m_Results.size(); i++)
			{
				const MeasureItem& res = m_Results.at(i);
				if (res.op.measureName == measureName && res.sourceViewId1 == sourceViewId1 && res.sourceViewId2 == sourceViewId2)
				{
					int plane = row % 4;
					if (res.op.hasResults[plane])
					{
						return QString("%1").arg(res.op.results[plane], 0, 'f', 2);
					}else
					{
						return QString("");
					}
				}
			}
		}

		break;
	case Qt::FontRole:
		if (row%4 == 3)
		{
			QFont boldFont;
			boldFont.setBold(true);
			return boldFont;
		}
		break;
	case Qt::BackgroundRole:
		if (row%4 == 3)
		{
			return QBrush(QColor(224, 192, 192));
		}
		break;
	}
	return QVariant();
}

QVariant MeasureResultsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	switch(role){
	case Qt::DisplayRole:
		{
			if (orientation == Qt::Horizontal) {
				switch (section)
				{
				case 0:
					return QString("1");
				case 1:
					return QString("2");
				case 2:
					return QString("3");
				}
			}else {
				QString header = m_MeasureNameRows.at(section/4);
				int plane = section%4;
				switch (plane)
				{
				case 0:
					header += "(Y)";
					break;
				case 1:
					header += "(U)";
					break;
				case 2:
					header += "(V)";
					break;
				}
				return header;
			}
		}
		break;
	case Qt::FontRole:
		if (orientation == Qt::Vertical) 
		{
			if (section%4 == 3)
			{
				QFont boldFont;
				boldFont.setBold(true);
				return boldFont;
			}
		}
		break;
	}

	return QVariant();

}

Qt::ItemFlags MeasureResultsModel::flags( const QModelIndex & index ) const
{
	int row = index.row();
	int col = index.column();
	unsigned int sourceViewId1 = m_SourceView1Ids.at(col);
	unsigned int sourceViewId2 = m_SourceView2Ids.at(col);
	const QString& measureName = m_MeasureNameRows.at(row/4);

	for (int i=0; i<m_Results.size(); i++)
	{
		const MeasureItem& res = m_Results.at(i);
		if (res.op.measureName == measureName && res.sourceViewId1 == sourceViewId1 && res.sourceViewId2 == sourceViewId2)
		{
			int plane = row % 4;
			if (res.op.hasResults[plane])
			{
				return Qt::ItemIsEnabled;
			}
		}
	}
	return 0;
}

void MeasureResultsModel::ResultsUpdated()
{
	emit dataChanged(index(0,0), index(8, 1));
}

MeasureWindow::MeasureWindow(VideoViewList* vvList, QWidget *parent, Qt::WFlags flags) : 
	QMainWindow(parent, flags), m_VideoViewList(vvList), m_ToolBar(new QToolBar(this)),
	m_ResultsTable(new QTableView(this)), m_ResultsModel(NULL), m_UpdateTimer(NULL),
	m_ShowDisortionMap(false)
{
	m_ToolBar->setMovable(false);
	m_ToolBar->setAllowedAreas(Qt::TopToolBarArea);
	m_ToolBar->setIconSize(QSize(22, 22));
	m_ToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_ToolBar->setFloatable(false);
	addToolBar(Qt::TopToolBarArea, m_ToolBar);

	setWindowFlags(Qt::Widget); 
	setCentralWidget(m_ResultsTable);
}

MeasureWindow::~MeasureWindow()
{
}

void MeasureWindow::showEvent( QShowEvent *event )
{
	UpdateRequest();
}

void MeasureWindow::hideEvent( QHideEvent *event )
{
	ClearAll();
}

QSize MeasureWindow::sizeHint() const
{
	/*QTableView* view = m_ResultsTable;
	QAbstractItemModel* m = view->model();

	QPainter painter;
	QFontMetrics fontMetrics = painter.fontMetrics();
	int columnWidth = fontMetrics.width("WWW.WW");*/
	int columnWidth = 50;
	
	return QSize(columnWidth*3, 250);
}

void MeasureWindow::on_button_Options_clicked()
{
	
}

void MeasureWindow::OnTimer()
{
	if (m_ResultsModel)
	{
		m_VideoViewList->GetProcessThread()->GetMeasureResults(m_MeasureItemList);
		m_ResultsModel->ResultsUpdated();
		m_ResultsTable->resizeColumnsToContents();
	}
}

void MeasureWindow::UpdateRequest()
{
	ClearAll();

	QSettings settings;
	QStringList distMap = settings.SETTINGS_GET_DIST_MAP();

	unsigned int sourceView1 = m_SourceList.at(0);
	for (int j=1; j<m_SourceList.size(); j++)
	{
		unsigned int sourceView2 = m_SourceList.at(j);

		const QList<PlugInInfo*>& plugins = GetHostImpl()->GetMeasurePluginList();
		for (int i=0; i<plugins.size(); i++)
		{
			PlugInInfo* info = plugins.at(i);

			MeasureItem req = {0};
			
			req.plugin = info;
			req.measure = info->plugin->NewMeasure(info->string);

			const MeasureCapabilities& caps = req.measure->GetCapabilities();
			for (int j=0; j<caps.measures.size(); j++)
			{
				unsigned int viewId = m_VideoViewList->NewVideoViewId();
				QString m = caps.measures.at(j).name;
				bool showDistMap = m_ShowDisortionMap?distMap.contains(m):false;

				if (showDistMap)
				{
					// Create new views
					m_VideoViewList->NewVideoViewCompare(m, viewId, sourceView1, sourceView2);
				}

				
				req.op.measureName = m;
				req.sourceViewId1 = sourceView1;
				req.sourceViewId2 = sourceView2;
				req.showDistortionMap = showDistMap;
				req.viewId = viewId;

				m_MeasureItemList.append(req);
			}
		}
	}

	m_ResultsModel = new MeasureResultsModel(this, m_MeasureItemList);
	m_VideoViewList->GetProcessThread()->SetMeasureRequests(m_MeasureItemList);
	m_ResultsTable->setModel(m_ResultsModel);
	// m_ResultsTable->setShowGrid(false);

	m_UpdateTimer = new QTimer(this);
	m_UpdateTimer->setInterval(200);
	connect(m_UpdateTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));
	m_UpdateTimer->start();

	PlaybackControl* c = m_VideoViewList->GetControl();
	PlaybackControl::Status status;
	c->GetStatus(&status);
	if (!status.isPlaying)
	{
		c->Seek(status.lastProcessPTS, false);
	}

}

void MeasureWindow::OnVideoViewSourceListChanged()
{
	m_SourceList = m_VideoViewList->GetSourceIDList();
	while (m_SourceList.size()>3)
	{
		m_SourceList.removeLast();
	}
	
	UpdateLabels();
	if (isVisible())
	{
		UpdateRequest();
	}
}

void MeasureWindow::ClearAll()
{
	if (m_UpdateTimer)
	{
		m_UpdateTimer ->stop();
		SAFE_DELETE(m_UpdateTimer);
	}

	// Clear request list from process threads
	m_VideoViewList->GetProcessThread()->SetMeasureRequests(QList<MeasureItem>());
	m_ResultsTable->setModel(NULL);
	
	QSet<Measure*> measureListDeleted;
	for (int i=0; i<m_MeasureItemList.size(); i++)
	{
		const MeasureItem& req = m_MeasureItemList.at(i);
		if (req.showDistortionMap)
		{
			m_VideoViewList->CloseVideoView(req.viewId);
		}

		if (!measureListDeleted.contains(req.measure))
		{
			measureListDeleted.insert(req.measure);

			req.plugin->plugin->ReleaseMeasure(req.measure);
		}
	}

	SAFE_DELETE(m_ResultsModel);
	m_MeasureItemList.clear();
}

void MeasureWindow::UpdateLabels()
{
	/*TextLabel* labels[] = {ui.label_Original, ui.label_Processed, ui.label_Processed_2};
	
	for (int i=0; i<sizeof(labels)/sizeof(labels[0]); i++)
	{
		if (m_SourceList.size()>i)
		{
			VideoView* vv = m_VideoViewList->find(m_SourceList.at(i));
			labels[i]->setText(vv->GetTitle());
			labels[i]->setToolTip(vv->GetTitle());
		}else
		{
			labels[i]->setText("");
			labels[i]->setToolTip("");
		}
	}

	for (int i=0; i<sizeof(labels)/sizeof(labels[0]); i++)
	{
		if (m_SourceList.size()>2)
		{
			labels[i]->show();
		}else
		{
			labels[i]->hide();
		}
	}*/
}

void MeasureWindow::OnShowDistortionMap( bool b)
{
	m_ShowDisortionMap = b;

	if (isVisible())
	{
		UpdateRequest();
	}
}
