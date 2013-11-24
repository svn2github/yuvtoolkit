TARGET   = YT_MeasuresBasic
QT      +=

DEFINES +=

SOURCES += YT_MeasuresBasic.cpp \
YT_MeasuresBasicPlugin.cpp

HEADERS += YT_MeasuresBasic.h \
YT_MeasuresBasicPlugin.h

! include( ../common.pri ) {
	error( common.pri not found )
}
