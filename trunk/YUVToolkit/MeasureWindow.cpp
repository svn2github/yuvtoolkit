#include "YT_Interface.h"

#include "MeasureWindow.h"
#include "VideoView.h"
#include "VideoViewList.h"


MeasureWindow::MeasureWindow(VideoViewList* vvList, QWidget *parent, Qt::WFlags flags) : 
	QWidget(parent, flags), m_VideoViewList(vvList)
{
	ui.setupUi(this);
	setMinimumWidth(120);
	setMinimumHeight(200);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	// connect(ui.originalList, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboIndexChanged(int)));
	// connect(ui.processedList, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboIndexChanged(int)));

	QTreeWidget* treeWidget = ui.tree_Results;
	treeWidget->setColumnCount(3);
	QStringList list;
	list.append("Type");
	list.append("Result 1");
	list.append("Result 2");
	treeWidget->setHeaderLabels(list);
	QTreeWidgetItem* psnrItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("PSNR")));
	treeWidget->addTopLevelItem(psnrItem);

	psnrItem->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("All"))));
	psnrItem->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Y"))));
	psnrItem->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("U"))));
	psnrItem->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("V"))));

	QTreeWidgetItem* mseItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("MSE")));
	treeWidget->addTopLevelItem(mseItem);

	mseItem->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("All"))));
	mseItem->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Y"))));
	mseItem->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("U"))));
	mseItem->addChild(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("V"))));

	treeWidget->resizeColumnToContents(0);
	treeWidget->expandAll();
	
	
	treeWidget = ui.tree_Distortion_Map;
	treeWidget->setColumnCount(1);
	
	QTreeWidgetItem* psnrMapItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("PSNR")));
	psnrMapItem->setFlags(psnrMapItem->flags() | Qt::ItemIsUserCheckable);
	psnrMapItem->setCheckState(0, Qt::Checked);
	treeWidget->addTopLevelItem(psnrMapItem);

	QTreeWidgetItem* mseMapItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("MSE")));
	mseMapItem->setFlags(mseMapItem->flags() | Qt::ItemIsUserCheckable);
	mseMapItem->setCheckState(0, Qt::Checked);
	treeWidget->addTopLevelItem(mseMapItem);
}

MeasureWindow::~MeasureWindow()
{
}

void MeasureWindow::showEvent( QShowEvent *event )
{
	/*
	for (int i=0; i<m_VideoViewList->size(); i++)
	{
		VideoView* vv = m_VideoViewList->at(i);
		Source* source = vv->GetSource();
		if (source)
		{
			ui.originalList->addItem(vv->GetTitle());
			ui.processedList->addItem(vv->GetTitle());
		}
	}

	if (ui.originalList->count()>1)
	{
		ui.originalList->setEnabled(true);
		ui.processedList->setEnabled(true);

		ui.originalList->setCurrentIndex(0);
		ui.processedList->setCurrentIndex(1);

		onComboIndexChanged(-1);
	}else
	{
		ui.originalList->setEnabled(false);
		ui.processedList->setEnabled(false);
	}*/
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
	return QSize(150,250);
}
