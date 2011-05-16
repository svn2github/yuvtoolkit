#include "../PlugIns/YT_Interface.h"

#include "MainWindow.h"
#include "YT_InterfaceImpl.h"

#include <QtGui/QApplication>
#include <objbase.h>

YT_HostImpl* g_Host = 0;
YT_Host* GetHost()
{
	return g_Host;
}

int main(int argc, char *argv[])
{
	// CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	QApplication app(argc, argv);
	app.setApplicationName("YUVToolkit");
	app.setOrganizationName("Yocto.net");
	app.setQuitOnLastWindowClosed(true);

	g_Host = new YT_HostImpl;
	QMainWindow* w = g_Host->NewMainWindow(argc, argv);
	
	w->show();
	int res = app.exec();

	delete g_Host;
	

	return res;
}
