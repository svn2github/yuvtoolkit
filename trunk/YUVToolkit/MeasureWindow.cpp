#include "YT_Interface.h"

#include "MeasureWindow.h"
#include "VideoView.h"
#include "VideoViewList.h"
#include "VideoQueue.h"

MeasureWindow::MeasureWindow(VideoViewList* vvList, QWidget *parent, Qt::WFlags flags) : 
	QWidget(parent, flags), m_VideoViewList(vvList)
{
	ui.setupUi(this);
	setMinimumWidth(320);
	setMinimumHeight(200);

	QTableWidget* tableWidget = ui.tableWidget;
	tableWidget->setRowCount(3);
	tableWidget->setColumnCount(2);
	tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("MSE"));
	tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("PSNR"));

	// tableWidget->setVerticalHeaderItem(0, new QTableWidgetItem("Overall"));
	tableWidget->setVerticalHeaderItem(0, new QTableWidgetItem("Y Component"));
	tableWidget->setVerticalHeaderItem(1, new QTableWidgetItem("U Component"));
	tableWidget->setVerticalHeaderItem(2, new QTableWidgetItem("V Component"));

	for (int i=0; i<2; i++)
	{
		for (int j=0; j<4; j++)
		{
			tableWidget->setItem(j, i, new QTableWidgetItem(""));
		}
	}

	connect(ui.originalList, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboIndexChanged(int)));
	connect(ui.processedList, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboIndexChanged(int)));
}

MeasureWindow::~MeasureWindow()
{
}

void MeasureWindow::showEvent( QShowEvent *event )
{
	for (int i=0; i<m_VideoViewList->size(); i++)
	{
		VideoView* vv = m_VideoViewList->at(i);
		YT_Source* source = vv->GetSource();
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
	}
}

void MeasureWindow::onComboIndexChanged( int )
{
	if (ui.originalList->count()<=1)
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

		YT_Measure* measure = plugInInfo->plugin->NewMeasure(plugInInfo->string);
		m_MeasureList.append(measure);

		YT_Source_Info origInfo, procInfo;
		m_Original->GetSource()->GetInfo(origInfo);
		m_Processed->GetSource()->GetInfo(procInfo);
		measure->GetSupportedModes(origInfo.format, procInfo.format, m_ViewOutItems, m_MeasureOutItems);

		for (int i=0; i<m_MeasureOutItems.size(); i++)
		{
			YT_Measure::YT_Measure_Item item = m_MeasureOutItems.at(i);
			m_OutputMeasureItems.insert(item, QVariant(-1));
		}
	}


}

void MeasureWindow::UpdateMeasure()
{
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

	YT_Measure* measure = m_MeasureList.at(0);

	measure->Process(origFrame->source, procFrame->source, m_OutputViewItems, m_OutputMeasureItems);
}

void MeasureWindow::UpdateMeasureWindow()
{
	if (!isVisible())	
	{
		// TODO; enable timer only when needed
		return;
	}

	QMapIterator<YT_Measure::YT_Measure_Item, QVariant> i(m_OutputMeasureItems);
	while (i.hasNext()) {
		i.next();
		// cout << i.key() << ": " << i.value() << endl;
		QTableWidgetItem* item = ui.tableWidget->item(i.key().plane, i.key().measureType);
		if (item)
		{
			item->setText(i.value().toString());
		}
	}
}