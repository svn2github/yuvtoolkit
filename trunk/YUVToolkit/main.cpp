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

	g_Host = new HostImpl(argc, argv);
	QMainWindow* w = g_Host->NewMainWindow();
	
	///////////////////////////////////////////////////
	//Generate a grid
	/*int gridSize = 16;
	int width = 640;
	int height = 480;
	QPen penDarkGray = QPen(Qt::darkGray, 0.1);

	QGraphicsScene scene(0,0,width,height);
	for (qreal x = gridSize; x < width; x += gridSize)
		scene.addLine(x, 0, x, height, penDarkGray);
	for (qreal y = gridSize; y < height; y += gridSize)
		scene.addLine(0, y, width, y, penDarkGray);

	QPixmap pixmap(width,height);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	scene.render(&painter);
	bool c = painter.end();

	bool b = pixmap.save("d:\\Temp\\scene.png");*/
	///////////////////////////////////////////////////

	w->show();
	QTimer::singleShot(100, g_Host, SLOT(Init()));

	int res = app.exec();

	g_Host->UnInitLogging();
	delete g_Host;
	

	return res;
}
