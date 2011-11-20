#include "Options.h"
#include "Settings.h"

Options::Options( QWidget *parent, Qt::WFlags flags ) : QDialog(parent, flags)
{
	ui.setupUi(this);

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
