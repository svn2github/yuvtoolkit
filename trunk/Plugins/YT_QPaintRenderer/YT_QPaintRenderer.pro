TARGET   = YT_QPaintRenderer

DEFINES += 

SOURCES += YT_QPaintRenderer.cpp

HEADERS += YT_QPaintRenderer.h
		
! include( ../common.pri ) {
	error( common.pri not found )
}
