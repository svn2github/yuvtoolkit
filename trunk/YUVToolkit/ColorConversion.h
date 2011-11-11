#ifndef COLOR_CONVERSION_H
#define COLOR_CONVERSION_H

class Frame;

void ColorConversion(const Frame& in, Frame& out);

// Return true if it is a supported format
bool IsFormatSupported(unsigned int fourcc);

// Return true if it is a format supported natively
bool IsNativeFormat(unsigned int fourcc);

COLOR_FORMAT GetNativeFormat(unsigned int fourcc);

COLOR_FORMAT GetFlippedColor(COLOR_FORMAT c);

#endif