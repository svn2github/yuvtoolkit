#ifndef COLOR_MAP_H
#define COLOR_MAP_H

#include "YT_Interface.h"


void CreateColorMap(FramePtr frame, DistMapPtr distMap, int width, int height, float upperRange, float lowerRange, bool biggerValueIsBetter);

#endif