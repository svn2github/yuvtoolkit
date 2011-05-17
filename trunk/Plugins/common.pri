QT       = core gui
TEMPLATE = lib

INCLUDEPATH += . ..

CONFIG(debug, debug|release) {
	DESTDIR = $$PWD/../Debug
} else {
	DESTDIR = $$PWD/../Release
}

macx {
	DYLIB_NEW = @executable_path/../MacOS
        DYLIB = lib$${TARGET}.1.0.0.dylib
        QMAKE_POST_LINK += mkdir -p $${DESTDIR}/YUVToolkit.app/Contents/MacOS;
        QMAKE_POST_LINK += cp -f $${DESTDIR}/$${DYLIB} $${DESTDIR}/YUVToolkit.app/Contents/MacOS;
        QMAKE_POST_LINK += install_name_tool -id $${DYLIB_NEW}/$${TARGET}.dylib $${DESTDIR}/YUVToolkit.app/Contents/MacOS/$${DYLIB};
	
	DYLIB_NEW = @executable_path/../Frameworks
        QMAKE_POST_LINK += install_name_tool -change QtGui.framework/Versions/4/QtGui $${DYLIB_NEW}/QtGui.framework/Versions/4/QtGui $${DESTDIR}/YUVToolkit.app/Contents/MacOS/$${DYLIB};
        QMAKE_POST_LINK += install_name_tool -change QtGui.framework/Versions/4/QtCore $${DYLIB_NEW}/QtGui.framework/Versions/4/QtCore $${DESTDIR}/YUVToolkit.app/Contents/MacOS/$${DYLIB};
}
