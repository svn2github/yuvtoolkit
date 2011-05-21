TARGET   = YT_GLRenderer
QT      += opengl

DEFINES += 

SOURCES += YT_GLRenderer.cpp

HEADERS += YT_GLRenderer.h
		
! include( ../common.pri ) {
	error( common.pri not found )
}
