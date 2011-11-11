TARGET   = YT_MeasuresBasic
QT      += 

DEFINES += 

SOURCES += YT_MeasuresBasic.cpp

HEADERS += YT_MeasuresBasic.h
		
! include( ../common.pri ) {
	error( common.pri not found )
}
