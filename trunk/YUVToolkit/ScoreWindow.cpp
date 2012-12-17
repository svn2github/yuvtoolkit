#include "YT_InterfaceImpl.h"
#include "ScoreWindow.h"

ScoreWindow::ScoreWindow( QWidget *parent, Qt::WFlags flags ) : QWidget(parent, flags)
{
	ui.setupUi(this);

	connect(ui.buttonNext, SIGNAL(clicked()), this, SIGNAL(onNext()));
	connect(ui.buttonPrevious, SIGNAL(clicked()), this, SIGNAL(onPrevious()));
	connect(ui.buttonFinish, SIGNAL(clicked()), this, SIGNAL(onFinish()));
}

ScoreWindow::~ScoreWindow()
{

}

void ScoreWindow::enableButtons( bool p, bool n, bool f)
{
	ui.buttonPrevious->setEnabled(p);
	ui.buttonNext->setEnabled(n);
	ui.buttonFinish->setEnabled(f);
}
