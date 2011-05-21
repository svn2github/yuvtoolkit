QT      += core gui
TEMPLATE = lib
CONFIG  += plugin

INCLUDEPATH += . ..

CONFIG(debug, debug|release) {
	DESTDIR = $$PWD/../Debug
} else {
	DESTDIR = $$PWD/../Release
}

macx {
	DYLIB_NEW = @executable_path/../MacOS
		DYLIB = lib$${TARGET}.dylib
        QMAKE_POST_LINK += mkdir -p $${DESTDIR}/YUVToolkit.app/Contents/MacOS;
        QMAKE_POST_LINK += cp -f $${DESTDIR}/$${DYLIB} $${DESTDIR}/YUVToolkit.app/Contents/MacOS;
        QMAKE_POST_LINK += install_name_tool -id $${DYLIB_NEW}/$${TARGET}.dylib $${DESTDIR}/YUVToolkit.app/Contents/MacOS/$${DYLIB};
	
	DYLIB_NEW = @executable_path/../Frameworks
        QMAKE_POST_LINK += install_name_tool -change QtGui.framework/Versions/4/QtGui $${DYLIB_NEW}/QtGui.framework/Versions/4/QtGui $${DESTDIR}/YUVToolkit.app/Contents/MacOS/$${DYLIB};
        QMAKE_POST_LINK += install_name_tool -change QtCore.framework/Versions/4/QtCore $${DYLIB_NEW}/QtCore.framework/Versions/4/QtCore $${DESTDIR}/YUVToolkit.app/Contents/MacOS/$${DYLIB};
        QMAKE_POST_LINK += install_name_tool -change QtOpenGL.framework/Versions/4/QtOpenGL $${DYLIB_NEW}/QtOpenGL.framework/Versions/4/QtOpenGL $${DESTDIR}/YUVToolkit.app/Contents/MacOS/$${DYLIB};
}
