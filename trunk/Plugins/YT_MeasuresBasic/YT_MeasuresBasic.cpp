#include "YT_MeasuresBasicPlugin.h"
#include "YT_MeasuresBasic.h"
#include "color_map.h"

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
/*
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
*/

const QList<MeasureCapability>& MeasuresBasic::GetCapabilities()
{
	if (m_Capabilities.isEmpty())
	{
		MeasureCapability cap = {0};
		cap.supportDistortionMap = true;

		cap.measureName = "MSE";
		m_Capabilities.append(cap);

		cap.measureName = "PSNR";
		m_Capabilities.append(cap);
	}

	return m_Capabilities;
}

void MeasuresBasic::Process(FramePtr source1, FramePtr source2, YUV_PLANE plane, const QList<MeasureOperation*>& operations)
{
	static double counter = 0;
	for (int i=0; i<operations.size(); i++)
	{
		MeasureOperation* op = operations[i];
		for (int i=0; i<PLANE_COUNT; i++)
		{
			op->results[i] = (counter+=1);
		}

		op->hasColorResult = true;
		op->hasPlaneResult = true;
	}
}

