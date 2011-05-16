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
    ./YT_InterfaceImpl.h
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
	 ../3rdparty/include
LIBS += -L"../3rdparty/lib" \
	-lswscale
win32:RC_FILE = YUVToolkit.rc
