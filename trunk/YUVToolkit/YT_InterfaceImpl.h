#ifndef YT_INTERFACE_IMPL_H
#define YT_INTERFACE_IMPL_H

#include "../Plugins/YT_Interface.h"
#include <QtGui>

class YT_HostImpl;

class YT_FormatImpl : public YT_Format
{
	YT_COLOR_FORMAT color;		    // color format listed in YT_COLOR_FORMAT or additional fourcc
	int width;
	int height;
	int stride[4];				// number of bytes per row in each plane
	
	size_t plane_size[4];
	bool format_changed;
	char name[4][32];
public:
	YT_FormatImpl();
	virtual ~YT_FormatImpl();
		// Get/Set color space
	virtual YT_COLOR_FORMAT Color() const;
	virtual void SetColor(YT_COLOR_FORMAT value);

	// Get/Set width
	virtual int Width() const;
	virtual void SetWidth(int value);

	// Get/Set height
	virtual int Height() const;
	virtual void SetHeight(int value);

	// Get/Set stride of each plane
	// number of bytes per row in each plane, plane 0-3
	virtual int Stride(int plane) const;
	virtual void SetStride(int plane, int value);
	virtual int* Stride() const;

	virtual size_t PlaneSize(int plane);
	virtual bool IsPlanar(int plane);
	virtual const char* PlaneName(int plane);
	virtual int PlaneWidth(int plane);
	virtual int PlaneHeight(int plane);

	// Convenience functions
	virtual bool operator== (const YT_Format &f);
	virtual bool operator!= (const YT_Format &f);
	virtual YT_Format& operator= (const YT_Format &f);
	virtual YT_Format& operator= (YT_Format &f);
};

class YT_FramePool;

class YT_FrameImpl : public YT_Frame
{
	unsigned char* data[4]; 	// pointers to the planes, for packed frame, only first one is used
	unsigned int pts; // presentation timestamp in miliseconds
	unsigned int frame_num;
	void* externData;
	
	unsigned char* allocated_data;
	size_t allocated_size;

	YT_Format_Ptr format;
	YT_FramePool* pool;

	void Deallocate();
public:
	YT_FrameImpl(YT_FramePool* p=NULL);
	virtual ~YT_FrameImpl();

	virtual const YT_Format_Ptr Format() const;
	virtual YT_Format_Ptr Format();
	virtual void SetFormat(const YT_Format_Ptr);

	// Get/Set data pointers of each plane
	virtual unsigned char* Data(int plane) const;
	virtual void SetData(int plane, unsigned char* value);
	virtual unsigned char** Data() const;

	virtual void SetExternData(void* data);
	virtual void* ExternData() const;

	// presentation timestamp in miliseconds
	virtual unsigned int PTS() const;
	virtual void SetPTS(unsigned int);

	virtual unsigned int FrameNumber() const;
	virtual void SetFrameNumber(unsigned int value);

	// Given the format, allocate the memory and populate Data
	YT_RESULT Allocate(); 
	// Reset the internal buffer, call me before changing the format
	YT_RESULT Reset();

	static void Recyle(YT_Frame *obj);
};

class YT_FramePool
{
	QList<YT_Frame*> m_Pool;
	QMutex m_Mutex;
public:
	YT_FramePool(unsigned int size);
	virtual ~YT_FramePool();

	YT_Frame_Ptr Get();

	void Recycle(YT_Frame* frame);
};

struct PlugInInfo
{
	YT_PlugIn* plugin;
	YT_PLUGIN_TYPE type;
	QString string;
};

class YT_HostImpl : public QObject, public YT_Host
{
	Q_OBJECT;

	QList<QMainWindow*> m_MainWindowList;
	QList<YT_PlugIn*> m_PlugInList;

	QList<PlugInInfo*> m_SourceList;
	QList<PlugInInfo*> m_RendererList;
	QList<PlugInInfo*> m_TransformList;
	QList<PlugInInfo*> m_MeasureList;

	volatile bool m_LoggingEnabled;
	QFile m_LogFile;
	QMutex m_MutexLogging;
public:
	YT_HostImpl();
	~YT_HostImpl();

	void InitLogging(); 
	void UnInitLogging();
	void EnableLogging(bool enable);
	bool IsLoggingEnabled();
	void OpenLoggingDirectory();

	virtual YT_Format_Ptr NewFormat();

	virtual YT_Frame_Ptr NewFrame();

	virtual YT_RESULT RegisterPlugin(YT_PlugIn*, YT_PLUGIN_TYPE, const QString& name);
	// virtual QMainWindow* GetMainWindow();

	QMainWindow* NewMainWindow(int argc, char *argv[]);

	virtual void Logging(void* ptr, YT_LOGGING_LEVELS level, const char* fmt, ...);

	/*
	YT_Source* NewSource(const QString file_ext);
	void ReleaseSource(YT_Source*);
*/
	YT_PlugIn* FindSourcePlugin(const QString file_ext);
	YT_PlugIn* FindRenderPlugin(const QString& type);

	const QList<PlugInInfo*>& GetRenderPluginList() {return m_RendererList; }
	const QList<PlugInInfo*>& GetTransformPluginList() {return m_TransformList; }
	const QList<PlugInInfo*>& GetMeasurePluginList() {return m_MeasureList; }
};

extern YT_HostImpl* GetHostImpl();

#endif // YT_INTERFACE_IMPL_H
