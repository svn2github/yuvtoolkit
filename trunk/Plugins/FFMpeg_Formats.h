#ifndef FFMPEG_FORMATS_H
#define FFMPEG_FORMATS_H

PixelFormat YT2FFMpegFormat(COLOR_FORMAT format)
{
	switch (format)
	{
	case RGB24:
		return PIX_FMT_RGB24;
	case RGBX32:
		return PIX_FMT_RGB32_1;
	case XRGB32:
		return PIX_FMT_RGB32;
	case Y800:
		return PIX_FMT_GRAY8;
	case BGR24:
		return PIX_FMT_BGR24;
	case BGRX32:
		return PIX_FMT_BGR32_1;
	case XBGR32:
		return PIX_FMT_BGR32;
	case RGB565:
		return PIX_FMT_RGB565;
	case BGR565:
		return PIX_FMT_BGR565;
	case I444:
	case YV24:
		return PIX_FMT_YUV444P;
	case I422:
	case YV16:
		return PIX_FMT_YUV422P;		
	case I420:
	case IYUV:
	case YV12:
	case IMC2:
	case IMC4:
		return PIX_FMT_YUV420P;
	case YUY2:
	case YUYV:
		return PIX_FMT_YUYV422;
	case UYVY:
		return PIX_FMT_UYVY422;
	case YVYU:
		return PIX_FMT_YUYV422;
	case NV12:
		return PIX_FMT_NV12;
	default:
		return PIX_FMT_NONE;
	}
}

COLOR_FORMAT FFMpeg2YTFormat(PixelFormat ffmpeg_format)
{
	switch (ffmpeg_format)
	{
	case PIX_FMT_RGB24:
		return RGB24;
	case PIX_FMT_RGB32_1:
		return RGBX32;
	case PIX_FMT_RGB32:
		return XRGB32;
	case PIX_FMT_GRAY8:
		return Y800;
	case PIX_FMT_YUV420P:
		return I420;
	case PIX_FMT_YUV444P:
		return I444;
	case PIX_FMT_YUV422P:
		return I422;
	case PIX_FMT_YUYV422:
		return YUY2;
	case PIX_FMT_UYVY422:
		return UYVY;
	case PIX_FMT_NV12:
		return NV12;
	case PIX_FMT_BGR24:
		return BGR24;
	case PIX_FMT_BGR32_1:
		return BGRX32;
	case PIX_FMT_BGR32:
		return XBGR32;
	case PIX_FMT_RGB565:
		return RGB565;
	case PIX_FMT_BGR565:
		return BGR565;
	default:
		return NODATA;
	}
}

#endif
