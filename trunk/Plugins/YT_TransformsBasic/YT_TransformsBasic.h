#ifndef TRANSFORMSBASIC_H
#define TRANSFORMSBASIC_H

#include "../YT_Interface.h"

class ShowYUVComponent : public QObject, public Transform
{
	Q_OBJECT;

	FramePtr m_EmptyFrame;

	void ReleaseBuffers();
	void ProcessPlane(FramePtr input, FramePtr output, int plane);

	QList<TransformCapability> m_Capabilities;
public:
	ShowYUVComponent(); 
	~ShowYUVComponent(); 

	virtual const QList<TransformCapability>& GetCapabilities();

	virtual void Process(FramePtr source1, FramePtr source2, unsigned int transformId, int plane, FramePtr result);

	virtual RESULT GetSupportedModes(FormatPtr sourceFormat, QList<QString>& outputNames, QList<QString>& statNames);
	virtual RESULT GetFormat(FormatPtr sourceFormat, const QString& outputName, FormatPtr outputFormat);
	virtual RESULT GetFormat(unsigned int transformId, int plane, FormatPtr sourceFormat, FormatPtr outputFormat);

	// Process
	virtual RESULT Process(const FramePtr input, QMap<QString, FramePtr>& outputs, QMap<QString, QVariant>& stats);

};

#endif // TRANSFORMSBASIC_H
