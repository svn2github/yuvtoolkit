TEMPLATE = subdirs

SUBDIRS  = \
		   Plugins/YT_QPaintRenderer \
		   Plugins/YTS_Raw

# build must be last:
CONFIG  += ordered
SUBDIRS += YUVToolkit
