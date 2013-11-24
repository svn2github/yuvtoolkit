TEMPLATE = subdirs

SUBDIRS  = \
    Plugins/YT_GLRenderer \
    Plugins/YT_MeasuresBasic \
    Plugins/YTS_Raw

equals($$QMAKE_CC, "cl") {
    message("Running with Visual C++.")
    SUBDIRS  = Plugins/YTR_D3D
}

# build must be last:
CONFIG  += ordered
SUBDIRS += YUVToolkit
