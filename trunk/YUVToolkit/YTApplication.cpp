#include "YT_Interface.h"
#include "MainWindow.h"
#include "YT_InterfaceImpl.h"
#include "YTApplication.h"

HostImpl* g_Host = 0;
Host* GetHost()
{
	return g_Host;
}

YTApplication::YTApplication(int & argc, char ** argv) : QApplication(argc, argv)
{
	setOrganizationName("yuvtoolkit.com");
	setOrganizationDomain("yuvtoolkit.com");
	setApplicationName("YUVToolkit");
	setQuitOnLastWindowClosed(true);

	g_Host = new HostImpl(argc, argv);

	qRegisterMetaType<FramePtr>("FramePtr");
	qRegisterMetaType<FrameList>("FrameList");
	qRegisterMetaType<UintList>("UintList");
	qRegisterMetaType<RectList>("RectList");
	qRegisterMetaType<FrameListPtr>("FrameListPtr");

	QMainWindow* w = g_Host->NewMainWindow();
	w->show();

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

	QTimer::singleShot(100, g_Host, SLOT(Init()));
}


YTApplication::~YTApplication()
{
	g_Host->UnInitLogging();
	SAFE_DELETE(g_Host);
}

bool YTApplication::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::FileOpen:
	{
		QStringList list(static_cast<QFileOpenEvent *>(
							 event)->file());
		g_Host->OpenFiles(list);

		return true;
	}
	default:
		return QApplication::event(event);
	}
}
