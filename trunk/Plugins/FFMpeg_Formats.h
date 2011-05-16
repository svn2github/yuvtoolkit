#ifndef FFMPEG_FORMATS_H
#define FFMPEG_FORMATS_H

PixelFormat YT_Format_to_FFMpeg_Format(YT_COLOR_FORMAT YT_format)
{
	switch (YT_format)
	{
	case YT_RGB24:
		return PIX_FMT_RGB24;
	case YT_RGBX32:
		return PIX_FMT_RGB32_1;
	case YT_XRGB32:
		return PIX_FMT_RGB32;
	case YT_GRAYSCALE8:
		return PIX_FMT_GRAY8;
	case YT_I420:
	case YT_IYUV:
		return PIX_FMT_YUV420P;
	case YT_YV12:
		return PIX_FMT_YUV420P;
	case YT_YUY2:
		return PIX_FMT_YUYV422;
	case YT_UYVY:
		return PIX_FMT_UYVY422;
	case YT_NV12:
		return PIX_FMT_NV12;
	default:
		return PIX_FMT_NONE;
	}
}

YT_COLOR_FORMAT FFMpeg_Format_to_YT_Format(PixelFormat ffmpeg_format)
{
	switch (ffmpeg_format)
	{
	case PIX_FMT_RGB24:
		return YT_RGB24;
	case PIX_FMT_RGB32_1:
		return YT_RGBX32;
	case PIX_FMT_RGB32:
		return YT_XRGB32;
	case PIX_FMT_GRAY8:
		return YT_GRAYSCALE8;
	case PIX_FMT_YUV420P:
		return YT_I420;
	case PIX_FMT_YUYV422:
		return YT_YUY2;
	case PIX_FMT_UYVY422:
		return YT_UYVY;
	case PIX_FMT_NV12:
		return YT_NV12;
	default:
		return YT_NODATA;
	}
}

#endif
