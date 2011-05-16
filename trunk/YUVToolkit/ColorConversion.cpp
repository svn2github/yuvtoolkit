#include "YT_interface.h"

#include "ColorConversion.h"
#include "YT_interfaceImpl.h"
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

void PrepareCC(YT_COLOR_FORMAT& color, unsigned char* (&data)[4])
{
	switch (color)
	{
	case YT_YV12:
		color = YT_I420;
		// SWAP
		data[3] = data[1];
		data[1] = data[2];
		data[2] = data[3];
		data[3] = 0;
		break;
	case YT_IYUV:
		color = YT_I420;
		break;
	}
}

void ColorConversion(const YT_Frame& in, YT_Frame& out)
{
	const YT_Format& format_in = in.Format();
	YT_Format& format_out = out.Format();

	int step_in[4];
	memcpy(step_in, format_in.Stride(), sizeof(step_in));

	unsigned char* data_in[4], *data_out[4];
	memcpy(data_in, in.Data(), sizeof(data_in));
	memcpy(data_out, out.Data(), sizeof(data_out));

	YT_COLOR_FORMAT color_in  = format_in.Color();
	YT_COLOR_FORMAT color_out = format_out.Color();

	
	PrepareCC(color_in, data_in);
	PrepareCC(color_out, data_out);

	bool ok = false;
	if (!ok)
	{
		// Try FFMpeg conversion if framewave doesn't support the conversion
		struct SwsContext *ctx = sws_getContext(format_in.Width(), format_in.Height(),
			YT_Format_to_FFMpeg_Format(color_in), format_out.Width(), format_out.Height(),
			YT_Format_to_FFMpeg_Format(color_out),
			SWS_BILINEAR, NULL,NULL,NULL );
		if (ctx)
		{
			if (sws_scale( ctx, data_in, step_in, 0, format_in.Height(), data_out, format_out.Stride()) == format_out.Height())
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
			for (int j=0; j<format_out.Stride(i)*format_out.Height(); j++)
			{
				data_out[i][j] = rand()*255/RAND_MAX;
			}
		}
	}
}
