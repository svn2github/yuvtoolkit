#ifndef YT_TRANSFORMSBASIC_H
#define YT_TRANSFORMSBASIC_H

#include "../YT_Interface.h"

class YT_ShowYUVComponent : public QObject, public YT_Transform
{
	Q_OBJECT;

	YT_Frame_Ptr m_EmptyFrame;

	void ReleaseBuffers();
	void ProcessPlane(YT_Frame_Ptr input, YT_Frame_Ptr output, int plane);
public:
	YT_ShowYUVComponent(); 
	~YT_ShowYUVComponent(); 

	virtual YT_RESULT GetSupportedModes(YT_Format_Ptr sourceFormat, QList<QString>& outputNames, QList<QString>& statNames);
	virtual YT_RESULT GetFormat(YT_Format_Ptr sourceFormat, const QString& outputName, YT_Format_Ptr outputFormat);

	// Process
	virtual YT_RESULT Process(const YT_Frame_Ptr input, QMap<QString, YT_Frame_Ptr>& outputs, QMap<QString, QVariant>& stats);

};

#endif // YT_TRANSFORMSBASIC_H
