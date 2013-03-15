#ifndef YTS_RAW_H
#define YTS_RAW_H

#include "../YT_Interface.h"
#include "RawFormatWidget.h"
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QMap>

#include <stdio.h>

class RawPlugin : public QObject, public YTPlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YTPlugIn);
public:
	virtual RESULT Init(Host*);

	virtual Source* NewSource(const QString& name);
	virtual void ReleaseSource(Source*);
};


#define RESOLUTION_COUNT 13
extern QString resolution_names[RESOLUTION_COUNT];

class YTS_Raw : public QObject, public Source
{
	Q_OBJECT;

public:
	YTS_Raw();
	~YTS_Raw();

	// For file source, return list of supported file types, like "avi", "yuv" etc
	// For capture source, return list of devices
	virtual RESULT EnumSupportedItems(char** items);

	// Create and destroy
	virtual RESULT Init(SourceCallback* callback, const QString& path);
	virtual RESULT UnInit();

	virtual RESULT GetFrame(FramePtr frame, unsigned int seekingPTS);

	virtual RESULT GetInfo( SourceInfo& info );
	virtual RESULT GetTimeStamps(QList<unsigned int>& timeStamps);
	virtual RESULT SetTimeStamps(QList<unsigned int> timeStamps);
	virtual unsigned int IndexToPTS(unsigned int frame_idx);
	
	virtual bool HasGUI();
	virtual QWidget* CreateGUI(QWidget* parent);

	void ReInit(const FormatPtr format, double FPS);
	const QString GetPath() {return m_Path;}
protected:
	unsigned int PTSToIndex(unsigned int PTS);
	unsigned int IndexToPTSInternal(unsigned int frame_idx);

	void InitInternal();
private:
	double			m_FPS;
	unsigned int	m_FrameIndex;
	
	unsigned int	m_NumFrames;
	unsigned int	m_Duration;
	QList<unsigned int> m_TimeStamps; // custom time stamps
	int             m_InsertFrame0; // first timestamp is not zero, insert frame 0

	FILE* m_File;

	FormatPtr m_Format;
	QString m_Path;
	RawFormatWidget* m_RawFormatWidget;

	SourceCallback* m_Callback;
	QMutex m_Mutex;
};

#endif // YTS_RAW_H
