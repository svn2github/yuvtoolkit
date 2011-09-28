#ifndef YTS_RAW_H
#define YTS_RAW_H

#include "../YT_Interface.h"
#include "RawFormatWidget.h"
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QMap>

#include <stdio.h>

class YT_RawPlugin : public QObject, public YT_PlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YT_PlugIn);
public:
	virtual YT_RESULT Init(YT_Host*);

	virtual YT_Source* NewSource(const QString& name);
	virtual void ReleaseSource(YT_Source*);
};

class YTS_Raw : public QObject, public YT_Source
{
	Q_OBJECT;

public:
	YTS_Raw();
	~YTS_Raw();

	// For file source, return list of supported file types, like "avi", "yuv" etc
	// For capture source, return list of devices
	virtual YT_RESULT EnumSupportedItems(char** items);

	// Create and destroy
	virtual YT_RESULT Init(const QString& path);
	virtual YT_RESULT UnInit();

	virtual YT_RESULT GetFrame(YT_Frame_Ptr frame, unsigned int PTS);

	virtual YT_RESULT GetInfo(YT_Source_Info& info);
	virtual unsigned int IndexToPTS(unsigned int frame_idx);
	virtual unsigned int SeekPTS(unsigned int pts);

	virtual bool HasGUI();
	virtual QWidget* CreateGUI(QWidget* parent);

	void ReInit(const YT_Format_Ptr format, double FPS);
	const QString GetPath() {return m_Path;}
protected:
	unsigned int PTSToIndex(unsigned int PTS);

	void InitInternal();
private:
	double			m_FPS;
	unsigned int	m_FrameIndex;
	
	unsigned int	m_NumFrames;
	unsigned int	m_Duration;

	FILE* m_File;

	YT_Format_Ptr m_Format;
	QString m_Path;
	RawFormatWidget* m_RawFormatWidget;

	QMutex m_Mutex;
};

#endif // YTS_RAW_H
