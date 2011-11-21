TARGET   = YTR_D3D
QT      += 

DEFINES += 

SOURCES += YTR_D3D.cpp \
	D3DWidget.cpp

HEADERS += YTR_D3D.h \
	D3DWidget.h

! include( ../common.pri ) {
	error( common.pri not found )
}
