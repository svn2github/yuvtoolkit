TARGET   = YTS_Raw

DEFINES += 

SOURCES += RawFormatWidget.cpp\
		   YTS_Raw.cpp

HEADERS += RawFormatWidget.h\
		   YTS_Raw.h
		
FORMS   += RawFormatWidget.ui

! include( ../common.pri ) {
	error( common.pri not found )
}
