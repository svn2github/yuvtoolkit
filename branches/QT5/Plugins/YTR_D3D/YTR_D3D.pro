equals($$QMAKE_CC, "cl") {
    message("Running with Visual C++.")

    TARGET   = YTR_D3D
    QT      +=

    DEFINES +=

    SOURCES += YTR_D3D.cpp \
            D3DWidget.cpp

    HEADERS += YTR_D3D.h \
            D3DWidget.h

    LIBS += -ld3d9

    ! include( ../common.pri ) {
            error( common.pri not found )
    }
} else {
    error("D3D renderer can only be built with Visual Studio.")
}
