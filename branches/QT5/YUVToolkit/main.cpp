#if defined(Q_WS_X11)
#	include <X11/Xlib.h>
#endif

#include "YTApplication.h"

int main(int argc, char *argv[])
{
#if defined(Q_WS_X11)
	XInitThreads();
#endif

	YTApplication app(argc, argv);

	int res = app.exec();

	return res;
}
