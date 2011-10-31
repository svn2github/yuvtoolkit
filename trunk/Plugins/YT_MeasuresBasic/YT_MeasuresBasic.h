#ifndef MEASURESBASIC_H
#define MEASURESBASIC_H

#include "../YT_Interface.h"

class MeasuresBasic : public QObject, public Transform
{
	Q_OBJECT;

	double ComputeMSE(FramePtr input1, FramePtr input2, FramePtr output, int plane);
	void AddMeasure(int m, FormatPtr sourceFormat, QList<MeasureItem>& items, bool addAllPlanes);
public:
	MeasuresBasic();
	~MeasuresBasic();

	virtual const QList<TransformCapabilities>& GetCapabilities();

	virtual RESULT GetFormat(unsigned int transformId, int plane, FormatPtr sourceFormat, FormatPtr outputFormat);

	virtual void Process(FramePtr source1, FramePtr source2, QList<TransformOperation>& operations);

	virtual RESULT GetMeasureString(MeasureItem item, FormatPtr sourceFormat1, FormatPtr sourceFormat2, QString& str);

	virtual RESULT GetSupportedModes(FormatPtr sourceFormat1, FormatPtr sourceFormat2, 
		QList<MeasureItem>& outputViewItems, QList<MeasureItem>& outputMeasureItems);

	// Process
	virtual RESULT Process(const FramePtr input1, const FramePtr input2, 
		QMap<MeasureItem, FramePtr>& outputViewItems,
		QMap<MeasureItem, QVariant>& outputMeasureItems);
};

#endif // MEASURESBASIC_H
