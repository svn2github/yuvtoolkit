QT       = core gui
TEMPLATE = lib

INCLUDEPATH += . ..

CONFIG(debug, debug|release) {
	DESTDIR = $$PWD/../Debug
} else {
	DESTDIR = $$PWD/../Release
}

macx {
	CONFIG(debug, debug|release) {
		DESTDIR = $$PWD/../Debug/YUVToolkit.app/Contents/MacOS
	} else {
		DESTDIR = $$PWD/../Release/YUVToolkit.app/Contents/MacOS
	}
}
