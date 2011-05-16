QT       += core gui script scripttools

TARGET = YUVToolkit
TEMPLATE = app

HEADERS += ./ColorConversion.h \
    ./VideoQueue.h \
    ./UI/ClickableSlider.h \
    ./Layout.h \
    ./MainWindow.h \
    ./RendererWidget.h \
    ./RenderThread.h \
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
    ./RendererWidget.cpp \
    ./RenderThread.cpp \
    ./SourceThread.cpp \
    ./VideoQueue.cpp \
    ./VideoView.cpp \
    ./VideoViewList.cpp \
    ./YT_InterfaceImpl.cpp
FORMS += ./MainWindow.ui
RESOURCES += YUVToolkit.qrc


INCLUDEPATH += . \
     ../Plugins \
     ../3rdparty/ffmpeg/include

win32 {
     LIBS += -L"$$PWD/../3rdparty/ffmpeg/lib_win32"
 }

macx {
     LIBS += -L"$$PWD/../3rdparty/ffmpeg/lib_osx"
	 ffmpeg.path = YUVToolkit.app/Contents/Frameworks
	 ffmpeg.files = $$PWD/../3rdparty/ffmpeg/lib_osxdata/*.dylib
	 INSTALLS += ffmpeg
 }

LIBS += -lswscale
win32:RC_FILE = YUVToolkit.rc
