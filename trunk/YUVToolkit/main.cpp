#include "YT_Interface.h"
#include "MainWindow.h"
#include "YT_InterfaceImpl.h"
#if defined(Q_WS_X11)
#	include <X11/Xlib.h>
#endif
#include <QtGui/QApplication>

YT_HostImpl* g_Host = 0;
YT_Host* GetHost()
{
	return g_Host;
}

int main(int argc, char *argv[])
{
#if defined(Q_WS_X11)
        XInitThreads();
#endif

	// CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	QApplication app(argc, argv);
	app.setApplicationName("YUVToolkit");
	app.setOrganizationName("Yocto.net");
	app.setQuitOnLastWindowClosed(true);

	qRegisterMetaType<YT_Frame_Ptr>("YT_Frame_Ptr");
	qRegisterMetaType<YT_Frame_List>("YT_Frame_List");
	qRegisterMetaType<UintList>("UintList");
	qRegisterMetaType<RectList>("RectList");

	g_Host = new YT_HostImpl;
	QMainWindow* w = g_Host->NewMainWindow(argc, argv);
	
	w->show();
	int res = app.exec();

	delete g_Host;
	

	return res;
}
