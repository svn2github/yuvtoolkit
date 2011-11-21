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

	QSettings settings;
	ui.check_Auto_Resize->setChecked(settings.SETTINGS_GET_AUTO_RESIZE());
}

Options::~Options()
{

}

#define SET_SETTING(name, val) if (settings.SETTINGS_GET_##name() != val) {settings.SETTINGS_SET_##name(val);}
void Options::OnAccepted()
{
	QSettings settings;

	SET_SETTING(AUTO_RESIZE, ui.check_Auto_Resize->isChecked());
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
