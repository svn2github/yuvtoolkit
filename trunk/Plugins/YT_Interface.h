#ifndef YT_INTERFACE_H
#define YT_INTERFACE_H

#include "sigslot.h"
#include <QtCore>
class QWidget;

#define INVALID_PTS		0xffffffff
#define NEXT_PTS		0xfffffffe
#define YT_FOURCC(a,b,c,d) (((unsigned int)a)|(((unsigned int)b)<<8)|(((unsigned int)c)<<16)|(((unsigned int)d)<<24)) 

typedef enum _YT_RESULT {
	YT_OK 					= 0x00000000,
	YT_END_OF_FILE          = 0x00000001,
	YT_ERROR 				= 0xffffffff,
	YT_E_WRONG_FORMAT		= 0xf0000001,
	YT_E_PTS_NOT_FOUND      = 0xf0000002,
	YT_E_RENDER_RESET       = 0xf0000002, // renderer needs reset
} YT_RESULT;

enum YT_COLOR_FORMAT {
	YT_NODATA = 0xffffffff,
	YT_GRAYSCALE8 = 8,
	YT_RGB24  = 24,
	YT_RGBX32 = 32,
	YT_XRGB32 = 321,
	YT_I420   = 0x30323449,
	YT_IYUV   = 0x56555949,
	YT_YV12   = 0x32315659,
	YT_YUY2   = 0x32595559,
	YT_UYVY   = 0x59565955,
	YT_NV12   = 0x3231564E,
};

class YT_Format
{
public:
	virtual ~YT_Format() {}

	// Get/Set color space
	virtual YT_COLOR_FORMAT Color() const = 0;
	virtual void SetColor(YT_COLOR_FORMAT value) = 0;

	// Get/Set width
	virtual int Width() const = 0;
	virtual void SetWidth(int value) = 0;

	// Get/Set height
	virtual int Height() const = 0;
	virtual void SetHeight(int value) = 0;

	// Get/Set stride of each plane
	// number of bytes per row in each plane, plane 0-3
	virtual int Stride(int plane) const = 0;
	virtual void SetStride(int plane, int value) = 0;
	virtual int* Stride() const = 0;

	// Get the data length of each plane
	virtual size_t PlaneSize(int plane) = 0;
	virtual bool IsPlanar(int plane) = 0;
	virtual const char* PlaneName(int plane) = 0;
	virtual int PlaneWidth(int plane) = 0;
	virtual int PlaneHeight(int plane) = 0;

	// Convenience functions
	virtual bool operator== (const YT_Format &f) = 0;
	virtual bool operator!= (const YT_Format &f) = 0;
	virtual YT_Format& operator= (const YT_Format &f) = 0;
	virtual YT_Format& operator= (YT_Format &f) = 0;
};

typedef YT_Format* YT_Format_Ptr;

class YT_Frame;

enum YT_PLUGIN_TYPE {
	YT_PLUGIN_UNKNOWN       = -1,
	YT_PLUGIN_SOURCE		= 0,
	YT_PLUGIN_RENDERER		= 1,
	YT_PLUGIN_TRANSFORM     = 2,
	YT_PLUGIN_MEASURE       = 3,
};

// YT_Host is implemented in the host application, provided to
// the plugin modules
class YT_PlugIn;

#define INFO_LOG(fmt, ...)  GetHost()->Logging(this, YT_Host::LL_INFO, fmt, ##__VA_ARGS__)
#define DEBUG_LOG(fmt, ...)  GetHost()->Logging(this, YT_Host::LL_DEBUG, fmt, ##__VA_ARGS__)
#define WARNING_LOG(fmt, ...)  GetHost()->Logging(this, YT_Host::LL_WARNING, fmt, ##__VA_ARGS__)
#define ERROR_LOG(fmt, ...)  GetHost()->Logging(this, YT_Host::LL_ERROR, fmt, ##__VA_ARGS__)


class YT_Host
{
public:
	virtual ~YT_Host() {}

	virtual YT_Frame* NewFrame() = 0;
	virtual void ReleaseFrame(YT_Frame*) = 0;

	virtual YT_Format* NewFormat() = 0;
	virtual void ReleaseFormat(YT_Format*) = 0;

	enum YT_LOGGING_LEVELS {
		LL_INFO    = 0,
		LL_DEBUG   = 1,
		LL_WARNING = 2,
		LL_ERROR   = 3
	};

	virtual void Logging(void* ptr, YT_LOGGING_LEVELS level, const char* fmt, ...) = 0;

	virtual YT_RESULT RegisterPlugin(YT_PlugIn*, YT_PLUGIN_TYPE, const QString& name) = 0;
};

extern YT_Host* GetHost();

class YT_Source;
class YT_Renderer;
class YT_Transform;
class YT_Measure;

class YT_PlugIn
{
public:
	virtual ~YT_PlugIn() {}

	virtual YT_RESULT Init(YT_Host*) = 0;

	virtual YT_Source* NewSource(const QString& name) {return NULL;}
	virtual void ReleaseSource(YT_Source*) {}

	virtual YT_Renderer* NewRenderer(QWidget* widget, const QString& name) {return NULL;}
	virtual void ReleaseRenderer(YT_Renderer*) {}

	virtual YT_Transform* NewTransform(const QString& name) {return NULL;}
	virtual void ReleaseTransform(YT_Transform*) {}

	virtual YT_Measure* NewMeasure(const QString& name) {return NULL;}
	virtual void ReleaseMeasure(YT_Measure*) {}
};

Q_DECLARE_INTERFACE(YT_PlugIn, "net.yocto.YUVToolkit.PlugIn/1.0")

class YT_Frame
{
public:
	virtual ~YT_Frame() {}

	virtual const YT_Format& Format() const = 0;
	virtual YT_Format& Format() = 0;
	virtual void SetFormat(const YT_Format&) = 0;
	
	// Get/Set data pointers of each plane
	virtual unsigned char* Data(int plane) const = 0;
	virtual void SetData(int plane, unsigned char* value) = 0;
	virtual unsigned char** Data() const = 0;

	virtual void SetExternData(void* data) = 0;
	virtual void* ExternData() const = 0;

	// presentation timestamp in miliseconds
	virtual unsigned int PTS() const = 0;
	virtual void SetPTS(unsigned int) = 0;

	virtual unsigned int FrameNumber() const = 0;
	virtual void SetFrameNumber(unsigned int value) = 0;

	// Given the format, allocate the memory and populate Data
	virtual YT_RESULT Allocate() = 0; 
	// Reset the internal buffer, call me before changing the format
	virtual YT_RESULT Reset() = 0;
};

typedef YT_Frame* YT_Frame_Ptr;

// Each module below should manage memory of its own
typedef enum _YT_SOURCE_TYPE {
	YT_FILE_SOURCE,
	YT_CAPTURE_SOURCE
} YT_SOURCE_TYPE;

typedef void* YT_HANDLE;

struct YT_Source_Info
{
	YT_Format* format;
	float fps;
	unsigned int num_frames;
	unsigned int duration; // in ms
};



// YT_Source produces source video stream
// can be Webcam, AVI eller raw files or maybe from network?
class YT_Source
{
public:
	virtual ~YT_Source() {}

	// For file source, return list of supported file types, like "avi", "yuv" etc
	// For capture source, return list of devices
	// virtual YT_RESULT EnumSupportedItems(char** items) = 0;

	// Create and destroy
	virtual YT_RESULT Init(const QString& path) = 0;
	virtual YT_RESULT UnInit() = 0;

	// if PTS != 0xFFFFFFFE, seek to PTS
	// PTS might be larger than the duration, then returns the last frame
	// Else get next frame
	virtual YT_RESULT GetFrame(YT_Frame_Ptr frame, unsigned int PTS=NEXT_PTS) = 0;

	// Requesting source info
	virtual YT_RESULT GetInfo(YT_Source_Info& info) = 0;

	virtual unsigned int IndexToPTS(unsigned int frame_idx) = 0;

	virtual bool HasGUI() = 0;
	virtual QWidget* CreateGUI(QWidget* parent) = 0;

	// Signal when GUI is needed and should be popped up
	sigslot::signal0<> GUINeeded;
	sigslot::signal1<unsigned int> SeekTo;
};


#define SET_RECT(rect, left, top, width, height) {rect[0]=left; rect[1]=top; rect[2]=left+width; rect[3]=top+height;}
#define COPY_RECT(rc, qrc) {rc[0]=qrc.left(); rc[1]=qrc.top(); rc[2]=qrc.left()+qrc.width(); rc[3]=qrc.top()+qrc.height();}

struct YT_Render_Frame
{
	YT_Frame_Ptr frame;

	long srcRect[4]; // left, top, right, bottom
	long dstRect[4]; // left, top, right, bottom	
};

// Render host is the parent of all renderers in one 
// window. Can have multiple renderers inside
class YT_Renderer
{
public:
	virtual ~YT_Renderer() {}

	// returns the widget pointer in case you need to show and position the widget in the render region
	virtual QWidget* GetWidget() = 0;

	virtual YT_RESULT RenderScene(QList<YT_Render_Frame>& frameList) = 0;	

	// Allocate render specific buffers
	virtual YT_RESULT Allocate(YT_Frame_Ptr& frame, YT_Format_Ptr sourceFormat) = 0;
	virtual YT_RESULT Deallocate(YT_Frame_Ptr frame) = 0;

	// Prepare YT_Frame_Ptr before using it
	virtual YT_RESULT GetFrame(YT_Frame_Ptr& frame) = 0;
	virtual YT_RESULT ReleaseFrame(YT_Frame_Ptr frame) = 0;

	virtual bool NeedReset() = 0;
	// If any function returns YT_E_RENDER_RESET, caller need to
	// 1. Release all render frames using ReleaseFrame(..)
	// 2. Deallocate all render frames using Deallocate(..)
	// 3. Call Reset()
	// 4. Reallocate frames again using Allocae(..)
	virtual YT_RESULT Reset() = 0;
};

class YT_Transform 
{
public:
	virtual ~YT_Transform() {}

	 // Returns what format that is supported	
	virtual YT_RESULT GetSupportedModes(YT_Format_Ptr sourceFormat, QList<QString>& outputNames, QList<QString>& statNames) = 0;

	virtual YT_RESULT GetFormat(YT_Format_Ptr sourceFormat, const QString& outputName, YT_Format_Ptr outputFormat) = 0;

	// Process
	virtual YT_RESULT Process(const YT_Frame_Ptr input, QMap<QString, YT_Frame_Ptr>& outputs, QMap<QString, QVariant>& stats) = 0;
};

class YT_Measure
{
public:
	virtual ~YT_Measure() {}

	// Returns what format that is supported	
	virtual YT_RESULT GetSupportedModes(YT_Format_Ptr sourceFormat, QList<QString>& outputNames, QList<QString>& measureNames) = 0;

	// Process
	virtual YT_RESULT Process(const YT_Frame_Ptr input1, const YT_Frame_Ptr input2, 
		const QList<QString>* outputNames, QMap<QString, YT_Frame_Ptr>* outputs, 
		const QList<QString>* measureNames, QMap<QString, QVariant>* measures) = 0;
};


// convenience macros
#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) {delete x; x = NULL;} }
#endif


#endif // YT_INTERFACE_H