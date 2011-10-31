#include "YT_MeasuresBasic.h"

enum MeasureType {MEASURE_MSE=0, MEASURE_PSNR};
char* measureString[] = {"MSE", "PSNR"};

MeasuresBasic::MeasuresBasic()
{
}

MeasuresBasic::~MeasuresBasic()
{
}

double MeasuresBasic::ComputeMSE( FramePtr input1, FramePtr input2, FramePtr output, int plane )
{
	if (output)
	{
		output->Reset();
		output->Format()->SetColor(I420);
		output->Format()->SetStride(0, 0);
		output->Format()->SetWidth(input1->Format()->PlaneWidth(plane));
		output->Format()->SetHeight(input1->Format()->PlaneHeight(plane));
		output->Format()->PlaneSize(0); // Update internal	
		output->Allocate();
	}

	double mse = 0;
	int frameSize = input1->Format()->PlaneSize(plane);	
	unsigned char* p1 = input1->Data(plane);
	unsigned char* p2 = input2->Data(plane);
	for (int i=0; i<frameSize; i++, p1++, p2++)
	{
		int diff = ((int)(*p1))-((int)(*p2));
		mse += diff*diff;

		if (output)
		{
			output->Data(0)[i] = (unsigned char)qMin<double>(mse, 255);
		}
	}
	mse /= frameSize;

	return mse;
}

RESULT MeasuresBasic::GetMeasureString( MeasureItem item, FormatPtr sourceFormat1, FormatPtr sourceFormat2, QString& str )
{
	str = measureString[item.measureType];
	if (item.plane>=0 && item.plane<=3)
	{
		str += " ";
		str += sourceFormat1->PlaneName(item.plane);
		str += " Component";
	}

	return OK;
}

void MeasuresBasic::AddMeasure(int m, FormatPtr sourceFormat, QList<MeasureItem>& items, bool addAllPlanes)
{
	MeasureItem item;
	item.measureType = m;
	if (addAllPlanes)
	{
		item.plane = ALL_PLANES;
		items.append(item);
	}

	for (int p=0; p<4; p++)
	{
		if (sourceFormat->IsPlanar(p))
		{
			item.plane = p;
			items.append(item);
		}
	}
}

RESULT MeasuresBasic::GetSupportedModes( FormatPtr sourceFormat1, FormatPtr sourceFormat2, 
											  QList<MeasureItem>& outputViewItems, QList<MeasureItem>& outputMeasureItems )
{
	if (sourceFormat1->Color() != sourceFormat2->Color() || 
		sourceFormat1->Width() != sourceFormat2->Width() ||
		sourceFormat1->Height() != sourceFormat2->Height())
	{
		// Measure not supported
		return OK;
	}

	outputViewItems.clear();	
	outputMeasureItems.clear();

	// AddMeasure(MEASURE_MSE, sourceFormat1, outputNames, false);

	for (int m=0; m<sizeof(measureString)/sizeof(char*); m++)
	{
		AddMeasure(m, sourceFormat1, outputMeasureItems, true);
	}

	return OK;
}

RESULT MeasuresBasic::Process( const FramePtr input1, const FramePtr input2, 
									QMap<MeasureItem, FramePtr>& outputViewItems,
									QMap<MeasureItem, QVariant>& outputMeasureItems )
{
	if (input1->Format() != input2->Format())
	{
		return E_UNKNOWN;
	}

	double mses[4] = {0,0,0,0};
	double psnr[4] = {0,0,0,0};
	MeasureItem item;
	for (int p=0; p<4; p++)
	{
		if (!input1->Format()->IsPlanar(p))
		{
			continue;
		}

		item.plane = p;
		FramePtr output;

		item.measureType = MEASURE_MSE;
		mses[p] = ComputeMSE(input1, input2, output, p);
		if (outputMeasureItems.contains(item))
		{
			outputMeasureItems[item].setValue(mses[p]);
		}

		item.measureType = MEASURE_PSNR;
		double mse_min = qMax(mses[p], 0.01);
		psnr[p] = 20.0*log10(255.0) - 10.0*log10(mse_min);
		if (outputMeasureItems.contains(item))
		{
			outputMeasureItems[item].setValue(psnr[p]);
		}
	}

	return OK;
}

void MeasuresBasic::Process( FramePtr source1, FramePtr source2, QList<TransformOperation>& operations )
{

}

const QList<TransformCapabilities>& MeasuresBasic::GetCapabilities()
{

}

RESULT MeasuresBasic::GetFormat( unsigned int transformId, int plane, FormatPtr sourceFormat, FormatPtr outputFormat )
{

}

