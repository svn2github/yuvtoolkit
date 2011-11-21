#ifndef INTERFACE_IMPL_H
#define INTERFACE_IMPL_H

#include "../Plugins/YT_Interface.h"
#include <QtGui>
#include <QVariant>

class HostImpl;

class FormatImpl : public Format
{
	COLOR_FORMAT color;		    // color format listed in COLOR_FORMAT or additional fourcc
	int width;
	int height;
	int stride[4];				// number of bytes per row in each plane

	size_t plane_size[4];
	bool format_changed;
	char name[4][32];
public:
	FormatImpl();
	virtual ~FormatImpl();
		// Get/Set color space
	virtual COLOR_FORMAT Color() const;
	virtual void SetColor(COLOR_FORMAT value);

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
	virtual bool operator== (const Format &f);
	virtual bool operator!= (const Format &f);
	virtual Format& operator= (const Format &f);
	virtual Format& operator= (Format &f);
};

class FramePool;

class FrameImpl : public Frame
{
	void Deallocate();
public:
	FrameImpl(FramePool* p=NULL);
	virtual ~FrameImpl();

	virtual const FormatPtr Format() const;
	virtual FormatPtr Format();
	virtual void SetFormat(const FormatPtr);

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

	virtual bool HasInfo(INFO_KEY) const;
	virtual QVariant Info(INFO_KEY) const;
	virtual void SetInfo(INFO_KEY, QVariant);

	// Given the format, allocate the memory and populate Data
	RESULT Allocate();
	// Reset the internal buffer, call me before changing the format
	RESULT Reset();

	static void Recyle(Frame *obj);
private:
	unsigned char* data[4]; 	// pointers to the planes, for packed frame, only first one is used
	unsigned int pts; // presentation timestamp in miliseconds
	unsigned int frame_num;
	void* externData;

	unsigned char* allocated_data;
	size_t allocated_size;

	FormatPtr format;
	FramePool* pool;

	QMap<INFO_KEY, QVariant> info;
};

class FramePool
{
	QList<Frame*> m_Pool;
	QMutex m_Mutex;
	bool m_CanGrow;
public:
	FramePool(unsigned int size, bool canGrow);
	virtual ~FramePool();

	FramePtr Get();
	void Recycle(Frame* frame);
	int Size();
};

struct PlugInInfo
{
	YTPlugIn* plugin;
	PLUGIN_TYPE type;
	QString string;
};

class MainWindow;
class HostImpl : public QObject, public Host
{
	Q_OBJECT;

	QList<MainWindow*> m_MainWindowList;
	QList<YTPlugIn*> m_PlugInList;

	QList<PlugInInfo*> m_SourceList;
	QList<PlugInInfo*> m_RendererList;
	QList<PlugInInfo*> m_TransformList;
	QList<PlugInInfo*> m_MeasureList;
	QList<FramePool*> m_FramePoolList;
	QList<FramePool*> m_FramePoolListGC;

	volatile bool m_LoggingEnabled;
	QFile m_LogFile;
	QMutex m_MutexLogging;

	QMutex m_MutexFrameListPool;
	QList<FrameList*> m_FrameListPool;
	QStringList m_InitFileList;

	QMap<QString, MeasureInfo> m_MeasureInfo;
	QStringList m_Measures;
public:
	HostImpl(int argc, char *argv[]);
	~HostImpl();

public slots:
	void Init();
public:
	void InitLogging();
	void UnInitLogging();
	void EnableLogging(bool enable);
	bool IsLoggingEnabled();
	void OpenLoggingDirectory();
	bool IsInited();
	void OpenFiles(QStringList);

	FramePool* NewFramePool(unsigned int size, bool canGrow);
	void ReleaseFramePool(FramePool*);

	FrameListPtr GetFrameList();
	void ReleaseFrameList(FrameList*);
	static void RecyleFrameList(FrameList*);

	const QStringList& GetMeasures();
	const MeasureInfo& GetMeasureInfo(QString measureName);

	virtual FormatPtr NewFormat();

	virtual FramePtr NewFrame();

	virtual RESULT RegisterPlugin(YTPlugIn*, PLUGIN_TYPE, const QString& name);
	// virtual QMainWindow* GetMainWindow();

	QMainWindow* NewMainWindow();

	virtual void Logging(void* ptr, LOGGING_LEVELS level, const char* fmt, ...);

	/*
	Source* NewSource(const QString file_ext);
	void ReleaseSource(Source*);
*/
	YTPlugIn* FindSourcePlugin(const QString file_ext);
	YTPlugIn* FindRenderPlugin(const QString& type);

	const QList<PlugInInfo*>& GetRenderPluginList() {return m_RendererList; }
	const QList<PlugInInfo*>& GetTransformPluginList() {return m_TransformList; }
	const QList<PlugInInfo*>& GetMeasurePluginList() {return m_MeasureList; }

private:
	void UpdateMeasureInfo();
};

extern HostImpl* GetHostImpl();

class PlaybackControl
{
public:
	struct Status
	{
		bool isPlaying;
		unsigned int lastDisplayPTS;
		unsigned int lastProcessPTS;
		unsigned int seekingPTS;
		unsigned int selectionFrom;
		unsigned int selectionTo;

		YUV_PLANE plane;
	};

	PlaybackControl();

	void GetStatus(Status* status);
	void Reset();

	void Play(bool play);
	void PlayPause();
	void Seek(unsigned int pts);
	void Seek(unsigned int pts, bool play);
	void ShowPlane(YUV_PLANE p);
	void SelectFrom();
	void SelectTo();
	void ClearSelection();

	void OnFrameProcessed(unsigned int pts, unsigned int seekingPTS);
	void OnFrameDisplayed(unsigned int pts, unsigned int seekingPTS);
private:
	Status m_Status;
	QMutex m_Mutex;
};

struct MeasureItem
{
	PlugInInfo* plugin;
	Measure* measure;
	unsigned int viewId;
	unsigned int sourceViewId1;
	unsigned int sourceViewId2;
	bool showDistortionMap;

	MeasureOperation op;
};

#endif // INTERFACE_IMPL_H
