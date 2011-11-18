QT       = core gui script scripttools opengl

TARGET = YUVToolkit
TEMPLATE = app

HEADERS += ./ColorConversion.h \
    ./VideoQueue.h \
    ./UI/ClickableSlider.h \
    ./Layout.h \
    ./MainWindow.h \
    ./MeasureWindow.h \
    ./Options.h \
    ./RendererWidget.h \
    ./RenderThread.h \
    ./ProcessThread.h \
    ./SourceThread.h \
    ./UI/TextLabel.h \
    ./VideoView.h \
    ./VideoViewList.h \
    ./YT_InterfaceImpl.h \
    ../Plugins/YT_Interface.h
SOURCES += \
    ./ColorConversion.cpp \
    ./Layout.cpp \
    ./main.cpp \
    ./MainWindow.cpp \
    ./MeasureWindow.cpp \
    ./Options.cpp \
    ./RendererWidget.cpp \
    ./RenderThread.cpp \
    ./SourceThread.cpp \
    ./ProcessThread.cpp \
    ./VideoView.cpp \
    ./VideoViewList.cpp \
    ./YT_InterfaceImpl.cpp
FORMS += ./MainWindow.ui \
     ./Options.ui
RESOURCES += YUVToolkit.qrc

INCLUDEPATH += . \
     ../Plugins \
     ../3rdparty/ffmpeg/include

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../Debug
} else {
    DESTDIR = $$PWD/../Release
}

VERSION_1 = $$cat(../Setup/VERSION_1)
VERSION_2 = $$cat(../Setup/VERSION_2)
VERSION_3 = $$cat(../Setup/VERSION_3)
VERSION_4 = $$cat(../Setup/VERSION_4)
VERSION = $${VERSION_1}.$${VERSION_2}.$${VERSION_3}.$${VERSION_4}

FFMPEG_DIR = $$PWD/../3rdparty/ffmpeg
QT_LIBS = Core Gui Script ScriptTools OpenGL
win32 {
    LIBS += -L"$${FFMPEG_DIR}/lib_win32"

    EXTRA_DLLS += \
        $${FFMPEG_DIR}/bin/swscale-0.dll \
        $${FFMPEG_DIR}/bin/avutil-50.dll \
        ${QTDIR}/bin/mingwm10.dll \
        ${QTDIR}/bin/libgcc*.dll \

    # Copy extra DLL files
    EXTRA_DLLS_WIN = $${EXTRA_DLLS}
    EXTRA_DLLS_WIN ~= s,/,\\,g
    DESTDIR_WIN = $${DESTDIR}
    DESTDIR_WIN ~= s,/,\\,g
    for(FILE,EXTRA_DLLS_WIN){
        QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\\n\\t))
    }

    # Copy QT files
    CONFIG(debug, debug|release) {
        for(QT_LIB,QT_LIBS){
            QMAKE_POST_LINK +=$$quote(cmd /c copy /y ${QTDIR}\\bin\\Qt$${QT_LIB}d4.dll $${DESTDIR_WIN}$$escape_expand(\\n\\t))
        }
    } else {
        for(QT_LIB,QT_LIBS){
            QMAKE_POST_LINK +=$$quote(cmd /c copy /y ${QTDIR}\\bin\\Qt$${QT_LIB}4.dll $${DESTDIR_WIN}$$escape_expand(\\n\\t))
        }
    }

    # Icon
    RC_FILE = YUVToolkit.rc
 }

macx {
    LIBS += -L"$$PWD/../3rdparty/ffmpeg/lib_osx"

    DYLIB_NEW = @executable_path/../Frameworks
    DYLIB_DIR = $${FFMPEG_DIR}/lib_osx
    DYLIBS = libswscale.dylib \
        libavutil.dylib

    QMAKE_POST_LINK += mkdir -p $${DESTDIR}/$${TARGET}.app/Contents/Frameworks;

    # Copy libs and update reference
    for(DYLIB,DYLIBS){
        QMAKE_POST_LINK += cp -f $${DYLIB_DIR}/$${DYLIB} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks;
        QMAKE_POST_LINK += install_name_tool -id $${DYLIB_NEW}/$${DYLIB} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks/$${DYLIB};
        QMAKE_POST_LINK += install_name_tool -change ../lib/$${DYLIB} $${DYLIB_NEW}/$${DYLIB} $${DESTDIR}/$${TARGET}.app/Contents/MacOS/$${TARGET};
        for(DYLIB2,DYLIBS){
            QMAKE_POST_LINK += install_name_tool -change ../lib/$${DYLIB2} $${DYLIB_NEW}/$${DYLIB2} $${DESTDIR}/$${TARGET}.app/Contents/Frameworks/$${DYLIB};
        }
    }

    QMAKE_POST_LINK += macdeployqt $${DESTDIR}/$${TARGET}.app -dmg;

    SETUPDIR = $$PWD/../Setup
    QMAKE_POST_LINK += mv $${DESTDIR}/$${TARGET}.dmg $${SETUPDIR}/$${TARGET}-$${VERSION}-`date +%Y%m%d_%H%M%S`.dmg;

    # Icon
    ICON = YUVToolkit.icns
 }

linux-g++ {
    LIBS += -lX11
}

LIBS += -lswscale
