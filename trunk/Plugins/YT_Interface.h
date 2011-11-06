#ifndef INTERFACE_H
#define INTERFACE_H

#include "sigslot.h"
#include <QtCore>
class QWidget;

#define INVALID_PTS		0xffffffff
#define FOURCC(a,b,c,d) (((unsigned int)a)|(((unsigned int)b)<<8)|(((unsigned int)c)<<16)|(((unsigned int)d)<<24)) 

enum RESULT {
	OK 					= 0x00000000,
	END_OF_FILE         = 0x00000001,
	E_UNKNOWN 			= 0xffffffff,
	E_WRONG_FORMAT		= 0xf0000001,
	E_PTS_NOT_FOUND     = 0xf0000002,
	E_RENDER_RESET      = 0xf0000002, // renderer needs reset
};

enum COLOR_FORMAT {
	NODATA = 0xffffffff,
	GRAYSCALE8 = 8,
	RGB24  = 24,
	RGBX32 = 32,
	XRGB32 = 321,
	I420   = 0x30323449,
	IYUV   = 0x56555949,
	YV12   = 0x32315659,
	YUY2   = 0x32595559,
	UYVY   = 0x59565955,
	NV12   = 0x3231564E,
};

enum INFO_KEY {
	VIEW_ID,          // unsigned int, view id
	IS_LAST_FRAME,    // bool, indicate that it is the last frame
	SEEKING_PTS,      // unsigned int, result of seeking pts
	NEXT_PTS,         // unsigned int, PTS of next frame, INVALID_PTS for last frame
	SRC_RECT,         // QRect, source rect for rendering
	DST_RECT,         // QRect, destination rect for rendering
};

enum YUV_PLANE {
	PLANE_Y = 0,
	PLANE_U = 1,
	PLANE_V = 2,
	PLANE_ALL = 3,
};

class Format
{
public:
	virtual ~Format() {}

	// Get/Set color space
	virtual COLOR_FORMAT Color() const = 0;
	virtual void SetColor(COLOR_FORMAT value) = 0;

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
	virtual bool operator== (const Format &f) = 0;
	virtual bool operator!= (const Format &f) = 0;
	virtual Format& operator= (const Format &f) = 0;
	virtual Format& operator= (Format &f) = 0;
};

typedef QSharedPointer<Format> FormatPtr;

class Frame
{
public:
	virtual ~Frame() {}

	virtual const FormatPtr Format() const = 0;
	virtual FormatPtr Format() = 0;
	virtual void SetFormat(const FormatPtr) = 0;

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

	virtual bool HasInfo(INFO_KEY) const = 0;
	virtual QVariant Info(INFO_KEY) const = 0;
	virtual void SetInfo(INFO_KEY, QVariant) = 0;

	// Given the format, allocate the memory and populate Data
	virtual RESULT Allocate() = 0; 
	// Reset the internal buffer, call me before changing the format
	virtual RESULT Reset() = 0;
};

typedef QSharedPointer<Frame> FramePtr;
typedef QList<FramePtr> FrameList;
typedef QSharedPointer<FrameList> FrameListPtr;
typedef QList<unsigned int> UintList;
typedef QList<QRect> RectList;

enum PLUGIN_TYPE {
	PLUGIN_UNKNOWN       = -1,
	PLUGIN_SOURCE		= 0,
	PLUGIN_RENDERER		= 1,
	PLUGIN_TRANSFORM     = 2,
	PLUGIN_MEASURE       = 3,
};

// Host is implemented in the host application, provided to
// the plugin modules
class YTPlugIn;

#define INFO_LOG(fmt, ...)  GetHost()->Logging(this, Host::LL_INFO, fmt, ##__VA_ARGS__)
#define DEBUG_LOG(fmt, ...)  GetHost()->Logging(this, Host::LL_DEBUG, fmt, ##__VA_ARGS__)
#define WARNING_LOG(fmt, ...)  GetHost()->Logging(this, Host::LL_WARNING, fmt, ##__VA_ARGS__)
#define ERROR_LOG(fmt, ...)  GetHost()->Logging(this, Host::LL_ERROR, fmt, ##__VA_ARGS__)


class Host
{
public:
	virtual ~Host() {}

	virtual FramePtr NewFrame() = 0;

	virtual FormatPtr NewFormat() = 0;

	enum LOGGING_LEVELS {
		LL_INFO    = 0,
		LL_DEBUG   = 1,
		LL_WARNING = 2,
		LL_ERROR   = 3
	};

	virtual void Logging(void* ptr, LOGGING_LEVELS level, const char* fmt, ...) = 0;

	virtual RESULT RegisterPlugin(YTPlugIn*, PLUGIN_TYPE, const QString& name) = 0;
};

extern Host* GetHost();

class Source;
class Renderer;
class Transform;
class Measure;

class YTPlugIn
{
public:
	virtual ~YTPlugIn() {}

	virtual RESULT Init(Host*) = 0;

	virtual Source* NewSource(const QString& name) {return NULL;}
	virtual void ReleaseSource(Source*) {}

	virtual Renderer* NewRenderer(QWidget* widget, const QString& name) {return NULL;}
	virtual void ReleaseRenderer(Renderer*) {}

	virtual Transform* NewTransform(const QString& name) {return NULL;}
	virtual void ReleaseTransform(Transform*) {}

	virtual Measure* NewMeasure(const QString& name) {return NULL;}
	virtual void ReleaseMeasure(Measure*) {}
};

Q_DECLARE_INTERFACE(YTPlugIn, "net.yocto.YUVToolkit.PlugIn/1.1")

// Each module below should manage memory of its own
enum SOURCE_TYPE {
	FILE_SOURCE,
	CAPTURE_SOURCE
};

typedef void* HANDLE;

struct SourceInfo
{
	FormatPtr format;
	float fps;
	unsigned int num_frames;
	unsigned int duration; // in ms
	unsigned int lastPTS; // PTS of last frame
};


// Source produces source video stream
// can be Webcam, AVI eller raw files or maybe from network?
class Source
{
public:
	virtual ~Source() {}

	// For file source, return list of supported file types, like "avi", "yuv" etc
	// For capture source, return list of devices
	// virtual RESULT EnumSupportedItems(char** items) = 0;

	// Create and destroy
	virtual RESULT Init(const QString& path) = 0;
	virtual RESULT UnInit() = 0;

	// if PTS != 0xFFFFFFFE, seek to PTS
	// PTS might be larger than the duration, then returns the last frame
	// Else get next frame
	virtual RESULT GetFrame(FramePtr frame, unsigned int seekingPts=INVALID_PTS) = 0;

	// Requesting source info
	virtual RESULT GetInfo(SourceInfo& info) = 0;

	// Convert frame index to PTS
	virtual unsigned int IndexToPTS(unsigned int frame_idx) = 0;

	// Convert seeking PTS to nearest frame PTS
	// This function is used to sync video of different FPS and irregular FPS
	virtual unsigned int SeekPTS(unsigned int pts) = 0;

	virtual bool HasGUI() = 0;
	virtual QWidget* CreateGUI(QWidget* parent) = 0;

	// Signal when GUI is needed and should be popped up
	sigslot::signal0<> GUINeeded;
};


#define SET_RECT(rect, left, top, width, height) {rect[0]=left; rect[1]=top; rect[2]=left+width; rect[3]=top+height;}
#define COPY_RECT(rc, qrc) {rc[0]=qrc.left(); rc[1]=qrc.top(); rc[2]=qrc.left()+qrc.width(); rc[3]=qrc.top()+qrc.height();}

// Render host is the parent of all renderers in one 
// window. Can have multiple renderers inside
class Renderer
{
public:
	virtual ~Renderer() {}

	// returns the widget pointer in case you need to show and position the widget in the render region
	virtual QWidget* GetWidget() = 0;

	// frames should be frames allocated by renderer
	// it should have SRC_RECT/DST_RECT properties
	virtual RESULT RenderScene(const FrameList& frames) = 0;
	   
	// Allocate render specific buffers
	virtual RESULT Allocate(FramePtr& frame, FormatPtr sourceFormat) = 0;
	virtual RESULT Deallocate(FramePtr frame) = 0;

	// Prepare FramePtr before using it
	virtual RESULT GetFrame(FramePtr& frame) = 0;
	virtual RESULT ReleaseFrame(FramePtr frame) = 0;

	virtual bool NeedReset() = 0;
	// If any function returns E_RENDER_RESET, caller need to
	// 1. Release all render frames using ReleaseFrame(..)
	// 2. Deallocate all render frames using Deallocate(..)
	// 3. Call Reset()
	// 4. Reallocate frames again using Allocae(..)
	virtual RESULT Reset() = 0;
};

struct TransformCapability
{
	unsigned int transformId;
	QString outputName;

	unsigned int inputColorsCount;
	COLOR_FORMAT inputColors[8];
	
	bool supportColor; // Can operate on all planes jointly
	bool supportPlanes; // Can operate on each plane separately
	bool need2Inputs;
};

class Transform 
{
public:
	virtual ~Transform() {}

	virtual const QList<TransformCapability>& GetCapabilities() = 0;

	virtual void Process(FramePtr source1, FramePtr source2, unsigned int transformId, int plane, FramePtr result) = 0;

	 // Returns what format that is supported	
	virtual RESULT GetSupportedModes(FormatPtr sourceFormat, QList<QString>& outputNames, QList<QString>& statNames) {return OK;}

	virtual RESULT GetFormat(FormatPtr sourceFormat, const QString& outputName, FormatPtr outputFormat) {return OK;}

	// Process
	virtual RESULT Process(const FramePtr input, QMap<QString, FramePtr>& outputs, QMap<QString, QVariant>& stats) {return OK;}
};

struct MeasureCapability
{
	unsigned int measureId;
	QString outputName;

	unsigned int inputColorsCount;
	COLOR_FORMAT inputColors[8];

	bool supportColor; // Can operate on all planes jointly
	bool supportPlanes; // Can operate on each plane separately
};

struct MeasureOperation
{
	unsigned int MeasureId;
	bool resultMask[5]; // mask to show the validity of the results
	double results[5];  // plane 0-3 + all plane
};


class Measure
{
public:
	virtual ~Measure() {}

	virtual const QList<MeasureCapability>& GetCapabilities() = 0;
	virtual void Process(FramePtr source1, FramePtr source2, QList<MeasureOperation>& operations) = 0;
};


// convenience macros
#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) {delete x; x = NULL;} }
#endif


#endif // INTERFACE_H