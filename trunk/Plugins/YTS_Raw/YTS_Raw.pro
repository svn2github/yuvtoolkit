QT      += core gui

TARGET = YTS_Raw
TEMPLATE = lib

DEFINES += 

SOURCES += RawFormatWidget.cpp\
		YTS_Raw.cpp

HEADERS += RawFormatWidget.h\
        YTS_Raw.h \
    RawFormatWidget.h
		
FORMS += RawFormatWidget.ui

INCLUDEPATH += . \
     ../Plugins \
     ../3rdparty/include
