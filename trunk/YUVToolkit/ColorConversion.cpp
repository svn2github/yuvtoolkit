#include "YT_Interface.h"
#include "ColorConversion.h"
#include "YT_InterfaceImpl.h"
#include <stdlib.h>
#include <memory.h>

#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#	undef _STDINT_H
#endif
#include <stdint.h>
extern "C" {
#include <libswscale/swscale.h>
}

#define MyMin(x,y)	((x<y)?x:y)

#include "FFMpeg_Formats.h"

COLOR_FORMAT GetFlippedColor(COLOR_FORMAT c)
{
	switch (c)
	{
	case YV12:
		return I420;
	case YV16:
		return I422;
	case YV24:
		return I444;
	default:
		return I420;
	}
}
void PrepareCC(COLOR_FORMAT& color, unsigned char* (&data)[4])
{
	switch (color)
	{
	case YV12:
	case YV16:
	case YV24:
		color = GetFlippedColor(color);
		// SWAP
		data[3] = data[1];
		data[1] = data[2];
		data[2] = data[3];
		data[3] = 0;
		break;
	case IYUV:
		color = I420;
		break;
	default:
		break;
	}
}

void ColorConversion(const Frame& in, Frame& out)
{
	const FormatPtr format_in = in.Format();
	FormatPtr format_out = out.Format();

	int step_in[4];
	memcpy(step_in, format_in->Stride(), sizeof(step_in));

	unsigned char* data_in[4], *data_out[4];
	memcpy(data_in, in.Data(), sizeof(data_in));
	memcpy(data_out, out.Data(), sizeof(data_out));

	COLOR_FORMAT color_in  = format_in->Color();
	COLOR_FORMAT color_out = format_out->Color();

	
	PrepareCC(color_in, data_in);
	PrepareCC(color_out, data_out);

	if (color_in == YVYU)
	{
		// Special case since ffmpeg converter doesn't have this by default
		// Swap chroma plans
		data_out[3] = data_out[1];
		data_out[1] = data_out[2];
		data_out[2] = data_out[3];
		data_out[3] = 0;
	}

	bool ok = false;
	if (!ok)
	{
		// Try FFMpeg conversion if framewave doesn't support the conversion
		struct SwsContext *ctx = sws_getContext(format_in->Width(), format_in->Height(),
			YT2FFMpegFormat(color_in), format_out->Width(), format_out->Height(),
			YT2FFMpegFormat(color_out),
			SWS_BILINEAR, NULL,NULL,NULL );
		if (ctx)
		{
			if (sws_scale( ctx, data_in, step_in, 0, format_in->Height(), data_out, format_out->Stride()) == format_out->Height())
			{
				ok = true;
			}
			sws_freeContext( ctx );
		}		
	}
	
	if (!ok)
	{
		// Fill with random data
		for (int i=0; i<4; i++)
		{
			for (int j=0; j<format_out->Stride(i)*format_out->Height(); j++)
			{
				data_out[i][j] = rand()*255/RAND_MAX;
			}
		}
	}

	out.SetPTS(in.PTS());
	out.SetFrameNumber(in.FrameNumber());
	for (int i=0; i<LAST_INFO_KEY; i++)
	{
		if (in.HasInfo((INFO_KEY)i))
		{
			out.SetInfo((INFO_KEY)i, in.Info((INFO_KEY)i));
		}
	}
}

bool IsNativeFormat( unsigned int fourcc )
{
	switch (fourcc)
	{
	case Y800:
	case I420:
	case I422:
	case I444:
		return true;
	}
	return false;
}

bool IsFormatSupported( unsigned int fourcc )
{
	switch (fourcc)
	{
	case Y800:
	case I420:
	case I422:
	case I444:
	case IYUV:
	case YV12:
	case YV16:
	case YV24:
	case YUY2:
	case UYVY:
	case YVYU:
	case YUYV:
	case NV12:
	case RGB24:
	case RGBX32:
	case XRGB32:
		return true;
	}
	return false;
}

COLOR_FORMAT GetNativeFormat( unsigned int fourcc )
{
	switch (fourcc)
	{
	case YV16:
	case YUY2:
	case UYVY:
	case YUYV:
	case YVYU:
		return I422;
	case YV12:
	case NV12:
	case IYUV:
		return I420;
	case YV24:
	case RGB24:
	case RGBX32:
	case XRGB32:
		return I444;
	}
	return I420;
}
