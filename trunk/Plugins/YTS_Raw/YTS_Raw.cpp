#include "YTS_Raw.h"
#include "RawFormatWidget.h"

#include <QtGui>
#include <math.h>
#include <assert.h>

QString resolution_names[RESOLUTION_COUNT] = 
{
	"QQVGA   (160 x 120)",
	"QCIF    (176 x 144)",
	"QVGA    (320 x 240)",
	"CIF     (352 x 288)",
	"VGA     (640 x 480)",	
	"480P    (720 x 480)",
	"4CIF    (704 x 576)",
	"576P    (720 x 576)", 
	"720P   (1280 x 720)",
	"1080P  (1920 x 1080)",
	"2160P  (3840 x 2160)",
	"4320P  (7680 x 4320)",
	"8640P (15360 x 8640)",
};


Q_EXPORT_PLUGIN2(YTS_Raw, RawPlugin)

#ifndef MyMin
#	define MyMin(x,y) ((x>y)?y:x)
#endif

#ifndef MyMax
#	define MyMax(x,y) ((x<y)?y:x)
#endif

Host* g_Host = 0;
Host* GetHost()
{
	return g_Host;
}


RESULT RawPlugin::Init( Host* host)
{
	g_Host = host;

	g_Host->RegisterPlugin(this, PLUGIN_SOURCE, QString("YUV/RGB Raw Video Files (*.yuv; *.rgb; *.raw)"));

	return OK;
}

Source* RawPlugin::NewSource( const QString& name )
{
	YTS_Raw* source = new YTS_Raw;

	return source;
}

void RawPlugin::ReleaseSource( Source* source)
{
	delete (YTS_Raw*)source;
}


YTS_Raw::YTS_Raw() : m_FPS(30), m_FrameIndex(0), m_InsertFrame0(0),
	m_NumFrames(0), m_Duration(0), m_RawFormatWidget(0), m_Callback(0)
{
}

YTS_Raw::~YTS_Raw()
{
}

RESULT YTS_Raw::EnumSupportedItems( char** items )
{
	return OK;
}

RESULT YTS_Raw::Init(SourceCallback* callback, const QString& path)
{
	m_Callback = callback;
	m_Format = GetHost()->NewFormat();
	
	m_Path = path;
	QString ext = path.right(4);

	bool unknownResolution = true;
	int pos = 0;
	m_Format->SetWidth(640);
	m_Format->SetHeight(480);
	m_Format->SetColor(I420);
	m_FPS = 30;

	// Parse resolution
	QRegExp rx;
	rx.setCaseSensitivity(Qt::CaseInsensitive);	
	rx.setPattern("(\\d\\d+)(X)(\\d\\d+)");
	
	pos = 0;
	while ((pos = rx.indexIn(path, pos)) !=-1) 
	{
		pos += rx.matchedLength();

		m_Format->SetWidth(rx.cap(1).toInt());
		m_Format->SetHeight(rx.cap(3).toInt());

		unknownResolution = false;
	}

	if (unknownResolution)
	{
		// Parse resolution in text
		QStringList resolutionList;
		QList<int> widthList;
		QList<int> heightList;
		for (int i=0; i<RESOLUTION_COUNT; i++)
		{
			rx.setPattern("([0-9a-zA-Z]+) +\\(([0-9]+) x ([0-9]+)\\)");
			if (rx.indexIn(resolution_names[i]) != -1)
			{
				resolutionList.append(rx.cap(1).toUpper());
				widthList.append(rx.cap(2).toInt());
				heightList.append(rx.cap(3).toInt());
			}			
		}

		rx.setPattern("(" + resolutionList.join("|") + ")");
		pos = 0;
		while ((pos = rx.indexIn(path, pos)) !=-1) 
		{
			pos += rx.matchedLength();

			int j = resolutionList.indexOf(rx.cap(1).toUpper());
			int w = widthList.at(j);
			int h = heightList.at(j);

			m_Format->SetWidth(w);
			m_Format->SetHeight(h);

			unknownResolution = false;
		}
	}

	// match 30Hz 30FPS
	rx.setPattern("([0-9]+)(\\.[0-9]+){0,1}(HZ|FPS)");
	pos = 0;
	while ((pos = rx.indexIn(path, pos)) !=-1) 
	{
		pos += rx.matchedLength();

		QString fps = rx.cap(1)+rx.cap(2);
		m_FPS = fps.toFloat();
	}

	rx.setPattern("I420|IYUV|UYVY|YUY2|YVYU|YUYV|YV12|NV12|Y800|RGB24|BGR24|RGBX32|XRGB32|BGRX32|XBGR32|RGBA32|ARGB32|BGRA32|ABGR32|RGB565|BGR565|GRAY8");
	
	pos = 0;
	while ((pos = rx.indexIn(path, pos)) !=-1)
	{
		pos += rx.matchedLength();

		QString fourcc(*(rx.capturedTexts().end()-1));
		fourcc = fourcc.toUpper();
		if (fourcc == "RGB24")
		{
			m_Format->SetColor(RGB24);
		}else if (fourcc == "BGR24")
		{
			m_Format->SetColor(BGR24);
		}else if (fourcc == "BGR24")
		{
			m_Format->SetColor(BGR24);
		}else if (fourcc == "RGBX32" || fourcc == "RGBA32")
		{
			m_Format->SetColor(RGBX32);
		}else if (fourcc == "XRGB32" || fourcc == "ARGB32")
		{
			m_Format->SetColor(XRGB32);
		}else if (fourcc == "BGRX32" || fourcc == "BGRA32")
		{
			m_Format->SetColor(BGRX32);
		}else if (fourcc == "XBGR32" || fourcc == "ABGR32")
		{
			m_Format->SetColor(XBGR32);
		}else if (fourcc == "RGB565")
		{
			m_Format->SetColor(RGB565);
		}else if (fourcc == "BGR565")
		{
			m_Format->SetColor(BGR565);
		}else if (fourcc == "GRAY8")
		{
			m_Format->SetColor(Y800);
		}else
		{
			COLOR_FORMAT cc = (COLOR_FORMAT) FOURCC(fourcc.at(0).toAscii(), 
				fourcc.at(1).toAscii(), 
				fourcc.at(2).toAscii(), 
				fourcc.at(3).toAscii());
			if (cc == IYUV)
			{
				cc = I420;
			}

			m_Format->SetColor(cc);
		}
	}

	InitInternal();

	m_Callback->ShowGui(this, unknownResolution);

	return OK;
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

	m_File = QSharedPointer<QFile>(new QFile(m_Path));
	m_File->open(QIODevice::ReadOnly);
	m_FrameIndex = 0;
}

RESULT YTS_Raw::GetInfo( SourceInfo& info )
{
	info.format = FormatPtr(GetHost()->NewFormat());
	*info.format = *m_Format;
	info.duration = m_Duration;
	info.num_frames = m_NumFrames+m_InsertFrame0;
	info.lastPTS = IndexToPTS(m_NumFrames-1);
	info.maxFps = m_FPS;

	if (m_TimeStamps.size())
	{
		info.duration = m_TimeStamps.last();
	}

	return OK;
}

RESULT YTS_Raw::UnInit()
{
	if (m_File)
	{
		m_File->close();
	}

	return OK;
}

RESULT YTS_Raw::GetFrame( FramePtr frame, unsigned int seekingPTS )
{
	QMutexLocker locker(&m_Mutex);

	if (!m_File)
	{
		return E_UNKNOWN;
	}

	if (seekingPTS < INVALID_PTS)
	{
		m_FrameIndex = PTSToIndex(seekingPTS);
		m_FrameIndex = MyMin(m_FrameIndex, m_NumFrames-1);

		WARNING_LOG("YTS_Raw GetFrame Seeking frame ptr %d index %d", seekingPTS, m_FrameIndex);
	}

	unsigned int frame_size = 0;
	for (int i=0; i<4; i++)
	{
		frame_size += m_Format->PlaneSize(i);
	}

	frame->SetFormat(m_Format);
	frame->Allocate();

	bool file_status = true;
	if (m_FrameIndex>0 || !m_InsertFrame0)
	{
		qint64 frameIdx = m_FrameIndex - m_InsertFrame0;
		qint64 readPos =  frame_size*frameIdx; 
		
		if (m_File->pos() != readPos)
		{
			file_status = m_File->seek(readPos);
		}

		for (int i=0; i<4 && file_status; i++)
		{
			unsigned int plane_size = m_Format->PlaneSize(i);

			if (plane_size>0)
			{
				file_status = m_File->read((char*)frame->Data(i), plane_size) == plane_size;
			}
		}
	}else
	{
		for (int i=0; i<4; i++)
		{
			unsigned int plane_size = m_Format->PlaneSize(i);

			if (plane_size>0)
			{
				memset(frame->Data(i), 0, plane_size);
			}
		}
	}

	
	if (file_status)
	{
		unsigned int pts = IndexToPTS(m_FrameIndex);
		frame->SetPTS(pts);
		frame->SetFrameNumber(m_FrameIndex);
		frame->SetInfo(IS_LAST_FRAME, m_FrameIndex == m_NumFrames-1);
		frame->SetInfo(SEEKING_PTS, seekingPTS);
		if (m_FrameIndex == m_NumFrames-1)
		{
			frame->SetInfo(NEXT_PTS, INVALID_PTS);
		}else
		{
			frame->SetInfo(NEXT_PTS, IndexToPTS(m_FrameIndex+1));
		}		

		m_FrameIndex++;

		return OK;
	}else
	{
		return END_OF_FILE;
	}
}


unsigned int YTS_Raw::IndexToPTS( unsigned int frame_idx )
{
	frame_idx = MyMin(frame_idx, m_NumFrames);
	if (m_TimeStamps.size())
	{
		return m_TimeStamps[frame_idx];
	}else
	{
		return (unsigned int)floor(1000.0*frame_idx/m_FPS);
	}
}

unsigned int YTS_Raw::IndexToPTSInternal( unsigned int frame_idx )
{
	frame_idx = MyMin(frame_idx, m_NumFrames-1);

	return (unsigned int)floor(1000.0*frame_idx/m_FPS);
}

unsigned int YTS_Raw::PTSToIndex( unsigned int PTS )
{
	unsigned int frame_idx = 0;
	if (m_TimeStamps.size())
	{
		QList<unsigned int>::iterator itr;
		unsigned int i;
		for (i=0, itr = m_TimeStamps.begin(); itr != m_TimeStamps.end(); ++itr, ++i)
		{
			if (itr == m_TimeStamps.end()-1)
			{
				frame_idx = i;
				break;
			}else if ((*itr)>PTS)
			{
				frame_idx = (i>0)?i-1:0;
				break;
			}
		}
	}else
	{
		frame_idx = (unsigned int)ceil(PTS*m_FPS/1000.0);
	}

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

void YTS_Raw::ReInit( const FormatPtr format, double FPS )
{
	QMutexLocker locker(&m_Mutex);
	
	// unsigned int pts = IndexToPTS(m_FrameIndex);

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

	m_Callback->ResolutionDurationChanged();
}

RESULT YTS_Raw::GetTimeStamps( QList<unsigned int>& timeStamps )
{
	timeStamps.clear();
	if (m_TimeStamps.size())
	{
		timeStamps.append(m_TimeStamps);
		timeStamps.removeLast();
	}else
	{		
		timeStamps.reserve(m_NumFrames);
		for (int i=0; i<m_NumFrames; i++) {
			timeStamps.append(IndexToPTS(i));
		}
	}
	
	return OK;
}

RESULT YTS_Raw::SetTimeStamps( QList<unsigned int> timeStamps )
{
	m_TimeStamps = timeStamps;
	m_InsertFrame0 = 0;

	if (m_TimeStamps.size()>0)
	{
		// Ensure that time stamp list is as big as the number of frames + 1
		// last one for duration

		while (m_TimeStamps.size()>m_NumFrames+1)
		{
			m_TimeStamps.removeLast();
		}

		// make sure it is non-decreasing
		for (int i = 1; i < m_TimeStamps.size(); ++i) 
		{
			if (m_TimeStamps.at(i) < m_TimeStamps.at(i-1))
			{
				m_TimeStamps[i] = m_TimeStamps[i-1];
			}
		}

		unsigned int lastTs = (m_TimeStamps.size())?m_TimeStamps.last():0;
		for (int i=1; m_TimeStamps.size()<m_NumFrames+1; i++)
		{
			m_TimeStamps.append(lastTs+IndexToPTSInternal(i));
		}

		if (!m_TimeStamps.startsWith(0))
		{
			m_InsertFrame0 = 1;
			m_TimeStamps.insert(m_TimeStamps.begin(), 0);
		}
	}

	m_Callback->ResolutionDurationChanged();

	return OK;
}
