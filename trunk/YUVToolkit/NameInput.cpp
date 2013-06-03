#include "NameInput.h"

NameInput::NameInput()
{
	ui.setupUi(this);
	this->setWindowFlags(0);
	this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint); 
}

NameInput::~NameInput()
{
}

void NameInput::accept()
{
	username = ui.lineEdit->text();
	if (username.isEmpty())
		{
			QMessageBox::information(this, "Warning...", "Dude! Please leave your name before starting the test!!", QMessageBox::Ok);
	}
	else
		this->hide();
}

QString NameInput::getUserName()
{
	return username;
}
