TEMPLATE = subdirs

SUBDIRS  = \
    Plugins/YT_GLRenderer \
    Plugins/YT_MeasuresBasic \
    Plugins/YTS_Raw

win32 {
SUBDIRS  = Plugins/YTR_D3D
}

# build must be last:
CONFIG  += ordered
SUBDIRS += YUVToolkit
