TEMPLATE = subdirs

SUBDIRS  = \
    Plugins/YT_GLRenderer \
    Plugins/YT_MeasuresBasic \
    Plugins/YTS_Raw

# build must be last:
CONFIG  += ordered
SUBDIRS += YUVToolkit
