#include "YTS_Raw.h"
#include "RawFormatWidget.h"

#include <QtGui>
#include <math.h>
#include <assert.h>

Q_EXPORT_PLUGIN2(YTS_Raw, YT_RawPlugin)

#ifndef MyMin
#	define MyMin(x,y) ((x>y)?y:x)
#endif

#ifndef MyMax
#	define MyMax(x,y) ((x<y)?y:x)
#endif

YT_Host* g_Host = 0;
YT_Host* GetHost()
{
	return g_Host;
}


YT_RESULT YT_RawPlugin::Init( YT_Host* host)
{
	g_Host = host;

	g_Host->RegisterPlugin(this, YT_PLUGIN_SOURCE, QString("Raw Video Files (*.yuv; *.raw)"));

	return YT_OK;
}

YT_Source* YT_RawPlugin::NewSource( const QString& name )
{
	YTS_Raw* source = new YTS_Raw;

	return source;
}

void YT_RawPlugin::ReleaseSource( YT_Source* source)
{
	delete (YTS_Raw*)source;
}


YTS_Raw::YTS_Raw() : m_RawFormatWidget(0), m_FrameIndex(0)
{
	m_FPS = 30;
}

YTS_Raw::~YTS_Raw()
{
}

YT_RESULT YTS_Raw::EnumSupportedItems( char** items )
{
	return YT_OK;
}

YT_RESULT YTS_Raw::Init( const QString& path)
{
	m_Format = GetHost()->NewFormat();
	
	m_Path = path;
	QString ext = path.right(4);

	bool unknownResolution = true;
	m_Format->SetWidth(640);
	m_Format->SetHeight(480);
	m_Format->SetColor(YT_I420);
	m_FPS = 30;

	// Parse resolution
	QRegExp rx;
	rx.setCaseSensitivity(Qt::CaseInsensitive);
	
	rx.setPattern("(\\d\\d+)(X)(\\d\\d+)");
	if (rx.indexIn(path) != -1)
	{
		// QStringList lst = (*(rx.capturedTexts().end()-1)).split("X");
		m_Format->SetWidth(rx.cap(1).toInt());
		m_Format->SetHeight(rx.cap(3).toInt());

		unknownResolution = false;
	}

	// match 30Hz 30FPS
	rx.setPattern("([0-9]+)(\\.[0-9]+){0,1}(HZ|FPS)");
	if (rx.indexIn(path) != -1)
	{
		QString fps = rx.cap(1)+rx.cap(2);
		m_FPS = fps.toFloat();
	}

	rx.setPattern("I420|IYUV|RGB24|UYVY|YUY2|YVYU|YV12|NV12|GRAY8");
	if (rx.indexIn(path) != -1)
	{
		QString fourcc(*(rx.capturedTexts().end()-1));
		fourcc = fourcc.toUpper();
		if (fourcc == "RGB24")
		{
			m_Format->SetColor(YT_RGB24);
		}else if (fourcc == "GRAY8")
		{
			m_Format->SetColor(YT_GRAYSCALE8);
		}else
		{
			YT_COLOR_FORMAT cc = (YT_COLOR_FORMAT) YT_FOURCC(fourcc.at(0).toAscii(), 
				fourcc.at(1).toAscii(), 
				fourcc.at(2).toAscii(), 
				fourcc.at(3).toAscii());

			m_Format->SetColor(cc);
		}
	}

	InitInternal();

	if (unknownResolution)
	{
		GUINeeded();
	}

	return YT_OK;
}


void YTS_Raw::InitInternal()
{
	unsigned int frame_size = 0;
	for (int i=0; i<4; i++)
	{
		frame_size += m_Format->PlaneSize(i);
	}

	QFileInfo file_info(m_Path);

	if (frame_size > 0)
	{
		m_NumFrames = file_info.size()/frame_size;
		m_NumFrames = MyMax(m_NumFrames, 1);
		
		m_Duration = IndexToPTS(m_NumFrames);
	}

	m_File = fopen(m_Path.toAscii(), "rb");
	m_FrameIndex = 0;
}

YT_RESULT YTS_Raw::GetInfo( YT_Source_Info& info )
{
	info.format = m_Format;
	info.duration = m_Duration;
	info.num_frames = m_NumFrames;
	info.lastPTS = IndexToPTS(m_NumFrames-1);
	info.fps = m_FPS;

	return YT_OK;
}

YT_RESULT YTS_Raw::UnInit()
{
	fclose(m_File);

	return YT_OK;
}

YT_RESULT YTS_Raw::GetFrame( YT_Frame_Ptr frame, unsigned int PTS )
{
	QMutexLocker locker(&m_Mutex);

	if (PTS == INVALID_PTS)
	{
		return YT_ERROR;
	}

	if (PTS < NEXT_PTS)
	{
		m_FrameIndex = PTSToIndex(PTS);
		m_FrameIndex = MyMin(m_FrameIndex, m_NumFrames-1);
	}

	unsigned int frame_size = 0;
	for (int i=0; i<4; i++)
	{
		frame_size += m_Format->PlaneSize(i);
	}

	int file_status = 0;
	if (ftell(m_File) != frame_size * m_FrameIndex) 
	{
		file_status = fseek(m_File, frame_size*m_FrameIndex, SEEK_SET);
	}

	frame->SetFormat(m_Format);
	frame->Allocate();

	for (int i=0; i<4 && file_status == 0; i++)
	{
		unsigned int plane_size = m_Format->PlaneSize(i);

		file_status = (fread(frame->Data(i), 1, plane_size, m_File ) == plane_size)? 0 : -1;
	}

	if (file_status == 0)
	{
		unsigned int pts = IndexToPTS(m_FrameIndex);
		frame->SetPTS(pts);
		frame->SetFrameNumber(m_FrameIndex);
		m_FrameIndex++;

		return YT_OK;
	}else
	{
		return YT_END_OF_FILE;
	}
}


unsigned int YTS_Raw::IndexToPTS( unsigned int frame_idx )
{
	frame_idx = MyMin(frame_idx, m_NumFrames-1);

	return (unsigned int)floor(1000.0*frame_idx/m_FPS);
}

unsigned int YTS_Raw::PTSToIndex( unsigned int PTS )
{
	unsigned int frame_idx = (unsigned int)ceil(PTS*m_FPS/1000.0);
	frame_idx = MyMin(frame_idx, m_NumFrames-1);

	return frame_idx;
}

bool YTS_Raw::HasGUI()
{
	return true;
}

QWidget* YTS_Raw::CreateGUI( QWidget* parent )
{
	RawFormatWidget* widget = new RawFormatWidget(this, parent);
	return widget;
}

void YTS_Raw::ReInit( const YT_Format_Ptr format, double FPS )
{
	QMutexLocker locker(&m_Mutex);
	
	unsigned int pts = IndexToPTS(m_FrameIndex);

	*m_Format = *format;
	m_FPS = FPS;

	unsigned int frame_size = 0;
	for (int i=0; i<4; i++)
	{
		frame_size += m_Format->PlaneSize(i);
	}

	QFileInfo file_info(m_Path);

	if (frame_size > 0)
	{
		m_NumFrames = file_info.size()/frame_size;

		m_Duration = IndexToPTS(m_NumFrames);
	}
}

unsigned int YTS_Raw::SeekPTS( unsigned int pts )
{
	unsigned int index = PTSToIndex(pts);	
	unsigned ptsNew = IndexToPTS(index);

	return ptsNew;
}
