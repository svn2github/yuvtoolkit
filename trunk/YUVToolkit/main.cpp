#include "YT_Interface.h"
#include "MainWindow.h"
#include "YT_InterfaceImpl.h"
#if defined(Q_WS_X11)
#	include <X11/Xlib.h>
#endif
#include <QtGui/QApplication>

HostImpl* g_Host = 0;
Host* GetHost()
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

	qRegisterMetaType<FramePtr>("FramePtr");
	qRegisterMetaType<FrameList>("FrameList");
	qRegisterMetaType<UintList>("UintList");
	qRegisterMetaType<RectList>("RectList");
	qRegisterMetaType<FrameListPtr>("FrameListPtr");
	
	g_Host = new HostImpl;
	QMainWindow* w = g_Host->NewMainWindow(argc, argv);
	
	w->show();
	int res = app.exec();

	delete g_Host;
	

	return res;
}
