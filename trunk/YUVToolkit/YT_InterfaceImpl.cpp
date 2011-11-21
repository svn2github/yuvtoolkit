#include "YT_Interface.h"
#include "YT_InterfaceImpl.h"
#include "MainWindow.h"
#include "Settings.h"

#include <stdint.h>
#include <string.h>

HostImpl* GetHostImpl()
{
	return (HostImpl*)GetHost();
}

FormatImpl::FormatImpl() : color(NODATA), width(0), height(0), format_changed(true)
{
	memset(stride, 0, sizeof(stride));
	memset(plane_size, 0, sizeof(plane_size));
}

FormatImpl::~FormatImpl()
{
}


COLOR_FORMAT FormatImpl::Color() const
{
	return color;
}

void FormatImpl::SetColor( COLOR_FORMAT value )
{
	color = value;
	format_changed = true;

	name[0][0] = name[1][0] = name[2][0] = name[3][0] = '\0';

	switch(color)
	{
	case I420:
	case I422:
	case I444:
	case IYUV:
		strcpy(name[0], "Y");
		strcpy(name[1], "U");
		strcpy(name[2], "V");
		break;
	case YV12:
	case YV16:
	case YV24:
		strcpy(name[0], "Y");
		strcpy(name[1], "V");
		strcpy(name[2], "U");
		break;
	case NV12:
		strcpy(name[0], "Y");
		strcpy(name[1], "UV");
		break;
	case YUY2:
		break;
	case UYVY:
		strcpy(name[0], "Y");
		break;
	case NODATA:
		break;
	case Y800:
		strcpy(name[0], "Y");
		break;
	case RGB24:
	case RGBX32:
	case XRGB32:
		break;
	}
}

int FormatImpl::Width() const
{
	return width;
}

void FormatImpl::SetWidth( int value )
{
	width = value;
	format_changed = true;
}

int FormatImpl::Height() const
{
	return height;
}

void FormatImpl::SetHeight( int value )
{
	height = value;
	format_changed = true;
}

int FormatImpl::Stride( int plane ) const
{
	if (plane>=0 && plane<4)
	{
		return stride[plane];
	}else
	{
		return 0;
	}
}

void FormatImpl::SetStride( int plane, int value )
{
	if (plane>=0 && plane<4)
	{
		stride[plane] = value;
		format_changed = true;
	}
}

int* FormatImpl::Stride() const
{
	return (int*)stride;
}


size_t FormatImpl::PlaneSize( int plane )
{
	if (format_changed)
	{
		if (stride[0] == 0)
		{
			stride[0] = stride[1] = stride[2] = stride[3] = 0;
			switch(color)
			{
			case I444:
			case I422:
			case YV16:
			case YV24:
				stride[0] = width;
				stride[1] = width;
				stride[2] = width;
				break;
			case I420:
			case IYUV:
			case YV12:
				stride[0] = width;
				stride[1] = width/2;
				stride[2] = width/2;
				break;
			case NV12:
				stride[0] = width;
				stride[1] = width;
				break;
			case YUY2:
			case UYVY:
				stride[0] = width*2;
				break;
			case RGB24:
				stride[0] = width*3;
				break;
			case XRGB32:
				stride[0] = width*4;
				break;
			case RGBX32:
				stride[0] = width*4;
				break;
			case Y800:
				stride[0] = width;
				break;
			case NODATA:
				break;
			}
		}

		int vstride[4] = {height, 0, 0, 0};
		switch(color)
		{
		case I444:
		case YV24:
			vstride[1] = height;
			vstride[2] = height;
			break;
		case YV16:
		case I422:
		case I420:
		case IYUV:
		case YV12:
		case NV12:
			vstride[1] = height/2;
			vstride[2] = height/2;
			break;
		case NODATA:
		case Y800:
		case RGB24:
		case RGBX32:
		case XRGB32:
		case YUY2:
		case UYVY:
			break;
		}

		for (int i=0; i<4; i++)
		{
			if (stride[i] != 0)
			{
				plane_size[i] = stride[i]*vstride[i];
			}
		}

		format_changed = false;
	}

	if (plane>=0 && plane<4)
	{
		return plane_size[plane];
	}else
	{
		return 0;
	}
}

bool FormatImpl::operator==( const Format &f )
{

	return (this->color == f.Color()) && (this->width == f.Width()) &&
		(this->height == f.Height()) && (this->stride[0] == f.Stride(0)) &&
		(this->stride[1] == f.Stride(1)) && (this->stride[2] == f.Stride(2)) &&
		(this->stride[3] == f.Stride(3));
}

bool FormatImpl::operator!=( const Format &f )
{
	return !(*this == f);
}

Format& FormatImpl::operator=( const Format &f )
{
	this->SetColor(f.Color());
	this->width = f.Width();
	this->height = f.Height();
	this->stride[0] = f.Stride(0);
	this->stride[1] = f.Stride(1);
	this->stride[2] = f.Stride(2);
	this->stride[3] = f.Stride(3);
	this->format_changed = true;

	return *this;
}

Format& FormatImpl::operator=( Format &f )
{
	*this = (const Format &)f;

	return *this;
}

bool FormatImpl::IsPlanar( int plane )
{
	switch(color)
	{
	case I444:
	case I422:
	case YV16:
	case YV24:
	case I420:
	case IYUV:
	case YV12:
		return plane<=2;
	case NV12:
	case Y800:
		return plane<=0;
	case YUY2:
	case UYVY:
		return false;
	default:
		return false;
	}
}

const char* FormatImpl::PlaneName( int plane )
{
	if (!IsPlanar(plane))
	{
		return NULL;
	}
	return name[plane];
}

int FormatImpl::PlaneWidth( int plane )
{
	if (!IsPlanar(plane))
	{
		return 0;
	}

	switch(color)
	{
	case Y800:
		if (plane == 0)
		{
			return width;
		}else
		{
			return 0;
		}
	case I444:
	case I422:
	case YV16:
	case YV24:
		return width;
	case I420:
	case IYUV:
	case YV12:
	case NV12:
		if (plane == 0)
		{
			return width;
		}else
		{
			return width/2;
		}
	default:
		return 0;
	}
}

int FormatImpl::PlaneHeight( int plane )
{
	if (!IsPlanar(plane))
	{
		return 0;
	}

	switch(color)
	{
	case Y800:
		if (plane == 0)
		{
			return height;
		}else
		{
			return 0;
		}
	case I444:
	case YV24:
		return height;
	case I422:
	case YV16:
	case I420:
	case IYUV:
	case YV12:
	case NV12:
		if (plane == 0)
		{
			return height;
		}else
		{
			return height/2;
		}
	default:
		return 0;
	}
}

FrameImpl::FrameImpl(FramePool* p) :
	pts(0), frame_num(0), externData(0),
	allocated_data(0), allocated_size(0),
	format(new FormatImpl), pool(p)
{
	memset(data, 0, sizeof(data));
}

FrameImpl::~FrameImpl()
{
	Deallocate();

	format.clear();
}

void FrameImpl::Deallocate()
{
	if (allocated_data)
	{
		delete []allocated_data;

		memset(data, 0, sizeof(data));

		allocated_data = 0;
		allocated_size = 0;
	}
}

FormatPtr FrameImpl::Format()
{
	return format;
}


void FrameImpl::SetFormat( const FormatPtr f)
{
	*format = *f;
}

const FormatPtr FrameImpl::Format() const
{
	return format;
}
unsigned char* FrameImpl::Data( int plane ) const
{
	if (plane>=0 && plane<4)
	{
		return data[plane];
	}else
	{
		return 0;
	}
}

void FrameImpl::SetData( int plane, unsigned char* value )
{
	// Set external buffer
	if (plane>=0 && plane<4)
	{
		if (allocated_data)
		{
			Deallocate();
		}

		data[plane] = value;
	}
}

unsigned char** FrameImpl::Data() const
{
	return (unsigned char**)data;
}

unsigned int FrameImpl::PTS() const
{
	return pts;
}

void FrameImpl::SetPTS( unsigned int value)
{
	pts = value;
}

unsigned int FrameImpl::FrameNumber() const
{
	return frame_num;
}

void FrameImpl::SetFrameNumber( unsigned int value )
{
	frame_num = value;
}

#define DEFAULT_ALIGNMENT	32
RESULT FrameImpl::Allocate()
{
	size_t data_length = format->PlaneSize(0)+format->PlaneSize(1)+
		format->PlaneSize(2)+format->PlaneSize(3);

	size_t align_margin = DEFAULT_ALIGNMENT*4;

	if (data_length+align_margin>allocated_size)
	{
		if (allocated_data)
		{
			Deallocate();
		}

		allocated_data = new unsigned char[data_length+align_margin];
		allocated_size = data_length+align_margin;
	}

	unsigned char* d = allocated_data;
	for (int i=0; i<4; i++)
	{
		while (((unsigned long)d) & (DEFAULT_ALIGNMENT-1))
			d ++;
		data[i] = d;

		d += format->PlaneSize(i);
	}

	return OK;
}

RESULT FrameImpl::Reset()
{
	Deallocate();

	for (int i=0; i<4; i++)
	{
		data[i] = 0;
	}

	externData = NULL;

	return OK;
}

void FrameImpl::SetExternData( void* data )
{
	externData = data;
}

void* FrameImpl::ExternData() const
{
	return externData;
}

void FrameImpl::Recyle( Frame *obj )
{
	FrameImpl* frame = static_cast<FrameImpl*> (obj);
	if (frame->pool)
	{
		frame->pool->Recycle(frame);
	}else
	{
		delete obj;
	}
}

QVariant FrameImpl::Info( INFO_KEY key) const
{
	return info.value(key);
}

void FrameImpl::SetInfo( INFO_KEY key, QVariant value)
{
	info.insert(key, value);
}

bool FrameImpl::HasInfo( INFO_KEY key ) const
{
	return info.contains(key);
}

FramePtr HostImpl::NewFrame()
{
	return FramePtr(new FrameImpl);
}

FormatPtr HostImpl::NewFormat()
{
	return FormatPtr(new FormatImpl);
}

HostImpl::HostImpl(int argc, char *argv[]) : m_LogFile(this)
{
	if (argc > 1)
	{
		for (int i=1; i<argc; i++)
		{
			m_InitFileList.append(argv[i]);
		}
	}
}

void HostImpl::Init()
{
	InitLogging();

	QDir pluginsDir(qApp->applicationDirPath());
	QStringList files;
#if defined(Q_WS_WIN)
	files.append(QString("YT*.dll"));
#elif defined(Q_WS_MACX)
	files.append(QString("libYT*.dylib"));
#elif defined(Q_OS_LINUX)
	files.append(QString("libYT*.so"));
#else
#	error Unsupported platform
#endif
	foreach (QString fileName, pluginsDir.entryList(files, QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));

		QObject *plugin = loader.instance();

		QString err(loader.errorString());

		YTPlugIn* ytPlugin = qobject_cast<YTPlugIn*>(plugin);

		if (ytPlugin)
		{
			if (ytPlugin->Init(this) == OK)
			{
				m_PlugInList.append(ytPlugin);
			}
		}
	}

	for (int i=0; i<m_MainWindowList.size(); i++)
	{
		MainWindow* win = m_MainWindowList.at(i);
		win->Init();
	}

	if (m_InitFileList.size()>0)
	{
		Q_ASSERT(m_MainWindowList.size()>0);
		MainWindow* win = m_MainWindowList.first();
		win->openFiles(m_InitFileList);
		m_InitFileList.clear();
	}
}

RESULT HostImpl::RegisterPlugin( YTPlugIn* plugin, PLUGIN_TYPE type, const QString& name )
{
	PlugInInfo* info = new PlugInInfo;

	info->plugin = plugin;
	info->string = name;
	info->type = type;

	switch (type)
	{
	case PLUGIN_SOURCE:
		m_SourceList.append(info);
		break;
	case PLUGIN_RENDERER:
		m_RendererList.append(info);
		break;
	case PLUGIN_TRANSFORM:
		m_TransformList.append(info);
		break;
	case PLUGIN_MEASURE:
		{
			m_MeasureList.append(info);
			Measure* m = plugin->NewMeasure(name);
			const MeasureCapabilities& cap = m->GetCapabilities();
			for (int i=0; i<cap.measures.size(); i++)
			{
				const MeasureInfo& info = cap.measures.at(i);

				m_MeasureInfo.insert(info.name, info);
			}
			
		}
		break;
	case PLUGIN_UNKNOWN:
		break;
	}

	return OK;
}

QMainWindow* HostImpl::NewMainWindow()
{
	MainWindow* win = new MainWindow;

	m_MainWindowList.append(win);

	return win;
}

HostImpl::~HostImpl()
{
	while (!m_MainWindowList.isEmpty())
	{
		delete m_MainWindowList.takeFirst();
	}
}
/*
Source* HostImpl::NewSource( const QString file_ext )
{
	PlugInInfo* info = m_SourceList.first();

	return info->plugin->NewSource(info->string);
}

void HostImpl::ReleaseSource( Source* )
{

}
*/
YTPlugIn* HostImpl::FindSourcePlugin( const QString file_ext )
{
	PlugInInfo* info = m_SourceList.first();

	return info->plugin;
}

YTPlugIn* HostImpl::FindRenderPlugin( const QString& type )
{
	for (int i = 0; i < m_RendererList.size(); ++i)
	{
		PlugInInfo* info = m_RendererList.at(i);
		if (info->string == type)
		{
			return info->plugin;
		}
	}

	if (m_RendererList.size()>0)
	{
		return m_RendererList.at(0)->plugin;
	}

	return NULL;
}

void HostImpl::Logging(void* ptr,  LOGGING_LEVELS level, const char* fmt, ... )
{
	if (m_LoggingEnabled)
	{
		QMutexLocker locker(&m_MutexLogging);
		if (m_LogFile.isOpen())
		{
			QString msg;

			va_list list;
			va_start(list, fmt);
			msg.vsprintf(fmt, list);

			QTextStream stream(&m_LogFile);

			stream << QTime::currentTime().toString();
			switch (level)
			{
				case LL_INFO:
					stream << " I ";
					break;
				case LL_DEBUG:
					stream << " D ";
					break;
				case LL_WARNING:
					stream << " W ";
					break;
				case LL_ERROR:
					stream << " E ";
					break;
			}

			showbase(hex(stream)) << (uintptr_t)ptr;
			stream << " " << msg << "\n";
		}
	}
}

void HostImpl::InitLogging()
{
	QSettings ini(QSettings::IniFormat, QSettings::UserScope,
		QCoreApplication::organizationName(),
		QCoreApplication::applicationName());
	QString dir = QFileInfo(ini.fileName()).absolutePath();

	if (!QDir().exists(dir))
	{
		QDir().mkdir(dir);
	}

	QSettings settings;
	if (QDir().exists(dir) && settings.SETTINGS_GET_LOGGING())
	{
		// Clean up old log files
		QDir qdir(dir);
		qdir.setFilter(QDir::Files | QDir::NoSymLinks);
		qdir.setSorting(QDir::Time | QDir::Reversed);

		QFileInfoList list = qdir.entryInfoList(QStringList("YUVToolkit-[1-9][0-9][0-1][0-9][0-3][0-9]-[0-2][0-9][0-6][0-9][0-6][0-9].log"));
		for (int i = 0; i <= list.size()-3; ++i)
		{
			// Keep 3 logs
			QFileInfo fileInfo = list.at(i);
			QFile::remove(fileInfo.filePath());
		}

		// Create new
		QString filename;

		QTextStream stream(&filename);
		stream << dir << "/" << "YUVToolkit-";
		stream << QDateTime::currentDateTime().toString("yyMMdd-hhmmss");
		stream << ".log";

		m_LogFile.setFileName(filename);
		m_LogFile.open(QIODevice::WriteOnly | QIODevice::Text);

		INFO_LOG("YUVToolkit Compiled %s %s", __DATE__, __TIME__);
	}

	m_LoggingEnabled = m_LogFile.isOpen();
}

void HostImpl::UnInitLogging()
{
	if (m_LogFile.isOpen())
	{
		m_LogFile.close();
	}
}

void HostImpl::EnableLogging( bool enable )
{
	QSettings settings;
	settings.SETTINGS_SET_LOGGING(enable);

	UnInitLogging();

	if (enable)
	{
		InitLogging();
	}
}

bool HostImpl::IsLoggingEnabled()
{
	return m_LoggingEnabled && m_LogFile.isOpen();
}

void HostImpl::OpenLoggingDirectory()
{
	QSettings ini(QSettings::IniFormat, QSettings::UserScope,
		QCoreApplication::organizationName(),
		QCoreApplication::applicationName());
	QString path = QFileInfo(ini.fileName()).absolutePath();

	QDesktopServices::openUrl(QUrl("file:///" + path));
}

FramePool* HostImpl::NewFramePool(unsigned int size, bool canGrow)
{
	FramePool* pool = new FramePool(size, canGrow);
	m_FramePoolList.append(pool);
	return pool;
}

void HostImpl::ReleaseFramePool(FramePool* pool)
{
	m_FramePoolListGC.append(pool);
	m_FramePoolList.removeAll(pool);

	/*
	// while (m_FramePool.Size() != BUFFER_COUNT)
	if (m_FramePool.Size() != BUFFER_COUNT)
	{
		WARNING_LOG("Waiting for frame pool to uninitialize (%s)... ", m_Path.toAscii());
		QThread::sleep(1);
	}*/
}

FrameListPtr HostImpl::GetFrameList()
{
	QMutexLocker locker(&m_MutexFrameListPool);
	if (m_FrameListPool.size()>0)
	{
		FrameList* list = m_FrameListPool.takeFirst();

		return FrameListPtr(list, HostImpl::RecyleFrameList);
	}
	return FrameListPtr(new FrameList, HostImpl::RecyleFrameList);
}

void HostImpl::ReleaseFrameList( FrameList* frameList)
{
	QMutexLocker locker(&m_MutexFrameListPool);
	if (frameList)
	{
		frameList->clear();
		m_FrameListPool.append(frameList);
	}
}

void HostImpl::RecyleFrameList( FrameList* frameList)
{
	GetHostImpl()->ReleaseFrameList(frameList);
}

FramePool::FramePool( unsigned int size, bool canGrow ) : m_CanGrow(canGrow)
{
	for (unsigned int i=0; i<size; i++)
	{
		m_Pool.append(new FrameImpl(this));
	}
}

FramePool::~FramePool()
{
	for (int i=0; i<m_Pool.size(); i++)
	{
		delete m_Pool.at(i);
	}
}

void FramePool::Recycle( Frame* frame )
{
	QMutexLocker locker(&m_Mutex);

	m_Pool.append(frame);
}

FramePtr FramePool::Get()
{
	if (m_Pool.size())
	{
		QMutexLocker locker(&m_Mutex);

		Frame* frame = m_Pool.takeFirst();
		return FramePtr(frame, FrameImpl::Recyle);
	}

	if (m_CanGrow)
	{
		return FramePtr(new FrameImpl(this));
	}else
	{
		return FramePtr(NULL);
	}
}

int FramePool::Size()
{
	return m_Pool.size();
}

PlaybackControl::PlaybackControl()
{
	Reset();
}


void PlaybackControl::Reset()
{
	QMutexLocker locker(&m_Mutex);

	m_Status.isPlaying = true;
	m_Status.lastDisplayPTS = 0;
	m_Status.lastProcessPTS = 0;
	m_Status.seekingPTS = INVALID_PTS;
	m_Status.plane = PLANE_COLOR;

	m_Status.selectionFrom = INVALID_PTS;
	m_Status.selectionTo = INVALID_PTS;
}


void PlaybackControl::Play( bool play )
{
	QMutexLocker locker(&m_Mutex);

	m_Status.isPlaying = play;
}

void PlaybackControl::PlayPause()
{
	QMutexLocker locker(&m_Mutex);

	m_Status.isPlaying = !m_Status.isPlaying;
}

void PlaybackControl::Seek( unsigned int pts )
{
	QMutexLocker locker(&m_Mutex);

	m_Status.seekingPTS = pts;

	WARNING_LOG("Seeking %d play %d", pts, m_Status.isPlaying);
}

void PlaybackControl::Seek( unsigned int pts, bool play )
{
	QMutexLocker locker(&m_Mutex);

	m_Status.seekingPTS = pts;
	m_Status.isPlaying = play;

	WARNING_LOG("Seeking %d play %d", pts, play);
}

void PlaybackControl::ShowPlane( YUV_PLANE p )
{
	QMutexLocker locker(&m_Mutex);

	m_Status.plane = p;
	WARNING_LOG("Show plane %d", p);
}

void PlaybackControl::GetStatus( Status* status )
{
	QMutexLocker locker(&m_Mutex);

	*status = m_Status;
}

void PlaybackControl::OnFrameProcessed( unsigned int pts, unsigned int seekingPTS )
{
	QMutexLocker locker(&m_Mutex);

	if (m_Status.seekingPTS == seekingPTS)
	{
		m_Status.seekingPTS = INVALID_PTS;
	}

	m_Status.lastProcessPTS = pts;
}

void PlaybackControl::OnFrameDisplayed( unsigned int pts, unsigned int seekingPTS )
{
	QMutexLocker locker(&m_Mutex);

	m_Status.lastDisplayPTS = pts;
}

void PlaybackControl::SelectFrom()
{
	QMutexLocker locker(&m_Mutex);

	m_Status.selectionFrom = m_Status.lastProcessPTS;

	if (m_Status.selectionTo <= m_Status.selectionFrom)
	{
		// Select to end
		m_Status.selectionTo = INVALID_PTS;
	}
}

void PlaybackControl::SelectTo()
{
	QMutexLocker locker(&m_Mutex);

	m_Status.selectionTo = m_Status.lastProcessPTS;
	if (m_Status.selectionFrom == INVALID_PTS || m_Status.selectionFrom>=m_Status.selectionTo)
	{
		m_Status.selectionFrom = 0;
	}
}

void PlaybackControl::ClearSelection()
{
	QMutexLocker locker(&m_Mutex);
	m_Status.selectionFrom = m_Status.selectionTo = INVALID_PTS;
}

bool HostImpl::IsInited()
{
	return m_PlugInList.size()>0 && m_MainWindowList.size()>0;
}

void HostImpl::OpenFiles(QStringList fileList)
{
	if (IsInited())
	{
		MainWindow* win = m_MainWindowList.at(0);
		win->openFiles(fileList);
	}else
	{
		m_InitFileList.append(fileList);
	}
}

const MeasureInfo& HostImpl::GetMeasureInfo( QString measureName )
{
	return m_MeasureInfo[measureName];
}

void HostImpl::UpdateMeasureInfo()
{
	
}
