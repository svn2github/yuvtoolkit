#include "YT_InterfaceImpl.h"
#include "Options.h"
#include "Settings.h"

#if defined(Q_WS_WIN)
#include <windows.h>
#include <shobjidl.h>
#endif

Options::Options( QWidget *parent, Qt::WFlags flags ) : QDialog(parent, flags)
{
	ui.setupUi(this);

	bool showFileAssociation = true;
#if defined(Q_WS_WIN)
	IApplicationAssociationRegistrationUI* pAARUI = NULL;
	HRESULT hr = ::CoCreateInstance( CLSID_ApplicationAssociationRegistrationUI, NULL, CLSCTX_INPROC, __uuidof( IApplicationAssociationRegistrationUI ), reinterpret_cast< void** >( &pAARUI ) );
	if ( SUCCEEDED( hr ) && pAARUI != NULL ) 
	{
		pAARUI->Release();
	} else
	{
		showFileAssociation = false;
	}
#else
	showFileAssociation = false;
#endif
	if (!showFileAssociation)
	{
		ui.button_File_Associations->setEnabled(false);
		ui.button_File_Associations->setVisible(false);
		ui.label_File_Associations->setVisible(false);
	}
	
	connect(this, SIGNAL(accepted()), this, SLOT(OnAccepted()));
	connect(ui.measureList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), 
		this, SLOT(OnMeasureSelected(QListWidgetItem*, QListWidgetItem*)));
	connect(ui.measureList, SIGNAL(itemChanged(QListWidgetItem*)), 
		this, SLOT(OnMeasureChanged(QListWidgetItem*)));

	QSettings settings;
	ui.check_Auto_Resize->setChecked(settings.SETTINGS_GET_AUTO_RESIZE());
	ui.check_Playback_Loop->setChecked(settings.SETTINGS_GET_PLAYBACK_LOOP());

	QStringList enabledMeasures = settings.SETTINGS_GET_MEASURES();
	QStringList distMaps = settings.SETTINGS_GET_DIST_MAP();
	const QStringList& measures = GetHostImpl()->GetMeasures();
	
	for (int i=0; i< measures.size(); i++)
	{
		const QString& m = measures.at(i);
		QListWidgetItem* item = new QListWidgetItem(m, ui.measureList);

		if (enabledMeasures.contains(m))
		{
			item->setCheckState(Qt::Checked);
		}else
		{
			item->setCheckState(Qt::Unchecked);
		}

		const MeasureInfo& info = GetHostImpl()->GetMeasureInfo(m);
		MeasureOptions o;
		o.lowerRange = info.lowerRange;
		o.upperRange = info.upperRange;
		o.showDistMap = distMaps.contains(m);
		o.hasDistMap = info.hasDistortionMap;
		o.info = info.name;

		m_MeasureOptions.insert(m, o);
	}

	if (measures.size()>0)
	{
		QListWidgetItem* item = ui.measureList->item(0);
		ui.measureList->setCurrentItem(item);
		LoadMeasureOptions(item);
	}else
	{
		LoadMeasureOptions(NULL);
	}
}

Options::~Options()
{

}

#define SET_SETTING(name, val) if (settings.SETTINGS_GET_##name() != val) {settings.SETTINGS_SET_##name(val);}
void Options::OnAccepted()
{
	SaveMeasureOptions(ui.measureList->currentItem());

	QStringList measureList, distMapList;
	for (int i=0; i<ui.measureList->count(); i++)
	{
		QListWidgetItem* item = ui.measureList->item(i);
		if (item->checkState() == Qt::Checked)
		{
			measureList.append(item->text());
			const MeasureOptions& o = m_MeasureOptions[item->text()];
			if (o.hasDistMap && o.showDistMap)
			{
				distMapList.append(item->text());
			}
		}
	}

	QSettings settings;

	SET_SETTING(AUTO_RESIZE, ui.check_Auto_Resize->isChecked());
	SET_SETTING(MEASURES, measureList);
	SET_SETTING(DIST_MAP, distMapList);
	SET_SETTING(PLAYBACK_LOOP, ui.check_Playback_Loop->isChecked());

	emit OptionChanged();
}

void Options::on_button_File_Associations_clicked()
{
#if defined(Q_WS_WIN)
	IApplicationAssociationRegistrationUI* pAARUI = NULL;
	HRESULT hr = ::CoCreateInstance( CLSID_ApplicationAssociationRegistrationUI, NULL, CLSCTX_INPROC, __uuidof( IApplicationAssociationRegistrationUI ), reinterpret_cast< void** >( &pAARUI ) );

	if ( SUCCEEDED( hr ) && pAARUI != NULL ) 
	{
		hr = pAARUI->LaunchAdvancedAssociationUI(L"YuvToolkit");
		pAARUI->Release();
	}
#endif
}

int Options::exec( int index )
{
	ui.tabWidget->setCurrentIndex(index);
	return QDialog::exec();
}

void Options::OnMeasureSelected(QListWidgetItem * current, QListWidgetItem * previous)
{
	SaveMeasureOptions(previous);
	LoadMeasureOptions(current);
}

void Options::LoadMeasureOptions( QListWidgetItem * current )
{
	ui.measureOptions->setVisible(current!=0);
	if (current)
	{
		ui.measureOptions->setEnabled(current->checkState() == Qt::Checked);

		QString measure = current->text();
		MeasureOptions& o = m_MeasureOptions[measure];

		ui.check_Show_Distortion_Map->setEnabled(o.hasDistMap);
		ui.check_Show_Distortion_Map->setChecked(o.showDistMap);
		ui.spin_Overshoot->setValue(o.upperRange);
		ui.spin_Undershoot->setValue(o.lowerRange);
		ui.label_Measure_Info->setText(o.info);
	}
}

void Options::OnMeasureChanged( QListWidgetItem * item )
{
	LoadMeasureOptions(ui.measureList->currentItem());
}

void Options::SaveMeasureOptions( QListWidgetItem * current )
{
	if (current)
	{
		QString measure = current->text();
		MeasureOptions& o = m_MeasureOptions[measure];

		o.showDistMap = ui.check_Show_Distortion_Map->isChecked();
	}
}
