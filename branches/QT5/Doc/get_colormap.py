from pylab import *

def c(v):
	return min(int(256*v),255)
map = get_cmap('jet')
print "unsigned int colormap[256] = {"
for i in range(0,256):
	print "    0x%02X%02X%02X%02X," %(c(map(i)[3]),c(map(i)[0]),c(map(i)[1]),c(map(i)[2]))
print "};"