#ifndef YT_MEASURESBASIC_H
#define YT_MEASURESBASIC_H

#include "../YT_Interface.h"

class YT_MeasuresBasic : public QObject, public YT_Measure
{
	Q_OBJECT;

	double ComputeMSE(YT_Frame_Ptr input1, YT_Frame_Ptr input2, YT_Frame_Ptr output, int plane);
	void AddMeasure(int m, YT_Format_Ptr sourceFormat, QList<YT_Measure_Item>& items, bool addAllPlanes);
public:
	YT_MeasuresBasic();
	~YT_MeasuresBasic();

	virtual YT_RESULT GetMeasureString(YT_Measure_Item item, YT_Format_Ptr sourceFormat1, YT_Format_Ptr sourceFormat2, QString& str);

	virtual YT_RESULT GetSupportedModes(YT_Format_Ptr sourceFormat1, YT_Format_Ptr sourceFormat2, 
		QList<YT_Measure_Item>& outputViewItems, QList<YT_Measure_Item>& outputMeasureItems);

	// Process
	virtual YT_RESULT Process(const YT_Frame_Ptr input1, const YT_Frame_Ptr input2, 
		QMap<YT_Measure_Item, YT_Frame_Ptr>& outputViewItems,
		QMap<YT_Measure_Item, QVariant>& outputMeasureItems);
};

#endif // YT_MEASURESBASIC_H
