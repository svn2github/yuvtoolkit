#include "YT_Interface.h"
#include "YT_InterfaceImpl.h"
#include "MainWindow.h"

#include <stdint.h>
#include <string.h>

YT_HostImpl* GetHostImpl()
{
	return (YT_HostImpl*)GetHost();
}

YT_FormatImpl::YT_FormatImpl() : color(YT_NODATA), width(0), height(0), format_changed(true)
{
	memset(stride, 0, sizeof(stride));
	memset(plane_size, 0, sizeof(plane_size));
}

YT_FormatImpl::~YT_FormatImpl()
{
}


YT_COLOR_FORMAT YT_FormatImpl::Color() const
{
	return color;
}

void YT_FormatImpl::SetColor( YT_COLOR_FORMAT value )
{
	color = value;
	format_changed = true;

	name[0][0] = name[1][0] = name[2][0] = name[3][0] = '\0';

	switch(color)
	{
	case YT_I420:
	case YT_IYUV:
		strcpy(name[0], "Y");
		strcpy(name[1], "U");
		strcpy(name[2], "V");
		break;
	case YT_YV12:
		strcpy(name[0], "Y");
		strcpy(name[1], "V");
		strcpy(name[2], "U");
		break;
	case YT_NV12:
		strcpy(name[0], "Y");
		strcpy(name[1], "UV");
		break;
	case YT_YUY2:
	case YT_UYVY:
	case YT_NODATA:
	case YT_GRAYSCALE8:
	case YT_RGB24:
	case YT_RGBX32:
	case YT_XRGB32:
		break;
	}
}

int YT_FormatImpl::Width() const
{
	return width;
}

void YT_FormatImpl::SetWidth( int value )
{
	width = value;
	format_changed = true;
}

int YT_FormatImpl::Height() const
{
	return height;
}

void YT_FormatImpl::SetHeight( int value )
{
	height = value;
	format_changed = true;
}

int YT_FormatImpl::Stride( int plane ) const
{
	if (plane>=0 && plane<4)
	{
		return stride[plane];
	}else
	{
		return 0;
	}
}

void YT_FormatImpl::SetStride( int plane, int value )
{
	if (plane>=0 && plane<4)
	{
		stride[plane] = value;
		format_changed = true;
	}
}

int* YT_FormatImpl::Stride() const
{
	return (int*)stride;
}


size_t YT_FormatImpl::PlaneSize( int plane )
{
	if (format_changed)
	{
		if (stride[0] == 0)
		{
			stride[0] = stride[1] = stride[2] = stride[3] = 0;
			switch(color)
			{
			case YT_I420:
			case YT_IYUV:
			case YT_YV12:
				stride[0] = width;
				stride[1] = width/2;
				stride[2] = width/2;
				break;
			case YT_NV12:
				stride[0] = width;
				stride[1] = width;
				break;
			case YT_YUY2:
			case YT_UYVY:
				stride[0] = width*2;
				break;
			case YT_RGB24:
				stride[0] = width*3;
				break;
			case YT_XRGB32:
				stride[0] = width*4;
				break;
			case YT_RGBX32:
				stride[0] = width*4;
				break;
			case YT_GRAYSCALE8:
				stride[0] = width;
				break;
			case YT_NODATA:
				break;
			}
		}

		int vstride[4] = {height, 0, 0, 0};
		switch(color)
		{
		case YT_I420:
		case YT_IYUV:
		case YT_YV12:
		case YT_NV12:
			vstride[1] = height/2;
			vstride[2] = height/2;
			break;
		case YT_NODATA:
		case YT_GRAYSCALE8:
		case YT_RGB24:
		case YT_RGBX32:
		case YT_XRGB32:
		case YT_YUY2:
		case YT_UYVY:
			break;
		}

		size_t data_length = 0;
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

bool YT_FormatImpl::operator==( const YT_Format &f )
{

	return (this->color == f.Color()) && (this->width == f.Width()) &&
		(this->height == f.Height()) && (this->stride[0] == f.Stride(0)) &&
		(this->stride[1] == f.Stride(1)) && (this->stride[2] == f.Stride(2)) && 
		(this->stride[3] == f.Stride(3));
}

bool YT_FormatImpl::operator!=( const YT_Format &f )
{
	return !(*this == f);
}

YT_Format& YT_FormatImpl::operator=( const YT_Format &f )
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

YT_Format& YT_FormatImpl::operator=( YT_Format &f )
{
	*this = (const YT_Format &)f;

	return *this;
}

bool YT_FormatImpl::IsPlanar( int plane )
{
	switch(color)
	{
	case YT_I420:
	case YT_IYUV:
	case YT_YV12:
		return plane<=2;
	case YT_NV12:
		return plane<=0;
	case YT_YUY2:
	case YT_UYVY:
		return false;
	}
	return false;
}

const char* YT_FormatImpl::PlaneName( int plane )
{
	if (!IsPlanar(plane))
	{
		return NULL;
	}
	return name[plane];
}

int YT_FormatImpl::PlaneWidth( int plane )
{
	if (!IsPlanar(plane))
	{
		return 0;
	}

	switch(color)
	{
	case YT_I420:
	case YT_IYUV:
	case YT_YV12:
	case YT_NV12:
		if (plane == 0)
		{
			return width;
		}else
		{
			return width/2;
		}
	}
	return 0;
}

int YT_FormatImpl::PlaneHeight( int plane )
{
	if (!IsPlanar(plane))
	{
		return 0;
	}

	switch(color)
	{
	case YT_I420:
	case YT_IYUV:
	case YT_YV12:
	case YT_NV12:
		if (plane == 0)
		{
			return height;
		}else
		{
			return height/2;
		}
	}
	return 0;
}

YT_FrameImpl::YT_FrameImpl() : pts(0), frame_num(0), externData(0),
	allocated_data(0), allocated_size(0),
	format(new YT_FormatImpl)
{
	memset(data, 0, sizeof(data));
}

YT_FrameImpl::~YT_FrameImpl()
{
	Deallocate();

	SAFE_DELETE(format);
}

void YT_FrameImpl::Deallocate()
{
	if (allocated_data)
	{
		delete []allocated_data;

		memset(data, 0, sizeof(data));

		allocated_data = 0;
		allocated_size = 0;
	}
}

YT_Format& YT_FrameImpl::Format()
{
	return *format;
}


void YT_FrameImpl::SetFormat( const YT_Format& f)
{
	*format = f;
}

const YT_Format& YT_FrameImpl::Format() const
{
	return *format;
}
unsigned char* YT_FrameImpl::Data( int plane ) const
{
	if (plane>=0 && plane<4)
	{
		return data[plane];
	}else
	{
		return 0;
	}
}

void YT_FrameImpl::SetData( int plane, unsigned char* value )
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

unsigned char** YT_FrameImpl::Data() const
{
	return (unsigned char**)data;
}

unsigned int YT_FrameImpl::PTS() const
{
	return pts;
}

void YT_FrameImpl::SetPTS( unsigned int value)
{
	pts = value;
}

unsigned int YT_FrameImpl::FrameNumber() const
{
	return frame_num;
}

void YT_FrameImpl::SetFrameNumber( unsigned int value )
{
	frame_num = value;
}

#define YT_DEFAULT_ALIGNMENT	32
YT_RESULT YT_FrameImpl::Allocate()
{
	size_t data_length = format->PlaneSize(0)+format->PlaneSize(1)+
		format->PlaneSize(2)+format->PlaneSize(3);

	size_t align_margin = YT_DEFAULT_ALIGNMENT*4;

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
		while (((unsigned long)d) & (YT_DEFAULT_ALIGNMENT-1))
			d ++;
		data[i] = d;
	
		d += format->PlaneSize(i);
	}

	return YT_OK;
}

YT_RESULT YT_FrameImpl::Reset()
{
	Deallocate();

	for (int i=0; i<4; i++)
	{
		data[i] = 0;
	}

	externData = NULL;

	return YT_OK;
}

void YT_FrameImpl::SetExternData( void* data )
{
	externData = data;
}

void* YT_FrameImpl::ExternData() const
{
	return externData;
}

YT_Frame* YT_HostImpl::NewFrame()
{
	return new YT_FrameImpl;
}

void YT_HostImpl::ReleaseFrame( YT_Frame* frame )
{
	delete frame;
}

YT_Format* YT_HostImpl::NewFormat()
{
	return new YT_FormatImpl;
}

void YT_HostImpl::ReleaseFormat( YT_Format* f)
{
	delete f;
}

YT_HostImpl::YT_HostImpl() : m_LogFile(this)
{
	QCoreApplication::setOrganizationName("YUVToolkit");
	QCoreApplication::setOrganizationDomain("YUVToolkit");
	QCoreApplication::setApplicationName("YUVToolkit");


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

		YT_PlugIn* yt_plugin = qobject_cast<YT_PlugIn*>(plugin);

		if (yt_plugin)
		{
			if (yt_plugin->Init(this) == YT_OK)
			{
				m_PlugInList.append(yt_plugin);
			}
		}
	}
}

YT_RESULT YT_HostImpl::RegisterPlugin( YT_PlugIn* plugin, YT_PLUGIN_TYPE type, const QString& name )
{
	PlugInInfo* info = new PlugInInfo;

	info->plugin = plugin;
	info->string = name;
	info->type = type;

	switch (type)
	{
	case YT_PLUGIN_SOURCE:
		m_SourceList.append(info);
		break;
	case YT_PLUGIN_RENDERER:
		m_RendererList.append(info);
		break;
	case YT_PLUGIN_TRANSFORM:
		m_TransformList.append(info);
		break;
	case YT_PLUGIN_MEASURE:
		m_MeasureList.append(info);
		break;
	case YT_PLUGIN_UNKNOWN:
		break;
	}

	return YT_OK;
}

QMainWindow* YT_HostImpl::NewMainWindow(int argc, char *argv[])
{
	MainWindow* win = new MainWindow;

	if (argc == 2)
	{
		QStringList fileList;
		fileList.append(argv[1]);
        win->openFiles(fileList);
		win->EnableButtons(true);
	}

	m_MainWindowList.append(win);

	return win;
}

YT_HostImpl::~YT_HostImpl()
{
	while (!m_MainWindowList.isEmpty())
	{
		delete m_MainWindowList.takeFirst();
	}
}
/*
YT_Source* YT_HostImpl::NewSource( const QString file_ext )
{
	PlugInInfo* info = m_SourceList.first();

	return info->plugin->NewSource(info->string);
}

void YT_HostImpl::ReleaseSource( YT_Source* )
{

}
*/
YT_PlugIn* YT_HostImpl::FindSourcePlugin( const QString file_ext )
{
	PlugInInfo* info = m_SourceList.first();

	return info->plugin;
}

YT_PlugIn* YT_HostImpl::FindRenderPlugin( const QString& type )
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

void YT_HostImpl::Logging(void* ptr,  YT_LOGGING_LEVELS level, const char* fmt, ... )
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

void YT_HostImpl::InitLogging()
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
	if (QDir().exists(dir) && settings.value("main/logging", false).toBool())
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

void YT_HostImpl::UnInitLogging()
{
	if (m_LogFile.isOpen())
	{
		m_LogFile.close();
	}
}

void YT_HostImpl::EnableLogging( bool enable )
{
	QSettings settings;
	settings.setValue("main/logging", enable);

	UnInitLogging();

	if (enable)
	{
		InitLogging();
	}
}

bool YT_HostImpl::IsLoggingEnabled()
{
	return m_LoggingEnabled && m_LogFile.isOpen();
}

void YT_HostImpl::OpenLoggingDirectory()
{
	QSettings ini(QSettings::IniFormat, QSettings::UserScope,
		QCoreApplication::organizationName(),
		QCoreApplication::applicationName());
	QString path = QFileInfo(ini.fileName()).absolutePath();

	QDesktopServices::openUrl(QUrl("file:///" + path));
}

