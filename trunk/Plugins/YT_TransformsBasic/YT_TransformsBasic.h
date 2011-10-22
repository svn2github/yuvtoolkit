#ifndef TRANSFORMSBASIC_H
#define TRANSFORMSBASIC_H

#include "../YT_Interface.h"

class ShowYUVComponent : public QObject, public Transform
{
	Q_OBJECT;

	FramePtr m_EmptyFrame;

	void ReleaseBuffers();
	void ProcessPlane(FramePtr input, FramePtr output, int plane);
public:
	ShowYUVComponent(); 
	~ShowYUVComponent(); 

	virtual RESULT GetSupportedModes(FormatPtr sourceFormat, QList<QString>& outputNames, QList<QString>& statNames);
	virtual RESULT GetFormat(FormatPtr sourceFormat, const QString& outputName, FormatPtr outputFormat);

	// Process
	virtual RESULT Process(const FramePtr input, QMap<QString, FramePtr>& outputs, QMap<QString, QVariant>& stats);

};

#endif // TRANSFORMSBASIC_H
