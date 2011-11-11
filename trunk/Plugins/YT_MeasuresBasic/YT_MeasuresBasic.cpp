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
	int idxMse = -1;
	int idxPsnr = -1;
	for (int i=0; i<operations.size(); i++)
	{
		MeasureOperation* op = operations[i];

		op->hasColorResult = true;
		op->hasPlaneResult = true;

		if (op->measureName == "MSE")
		{
			idxMse = i;
		}else if (op->measureName == "PSNR")
		{
			idxPsnr = i;
		}else
		{
			op->hasColorResult = false;
			op->hasPlaneResult = false;
		}
	}

	MeasureOperation* opMse = operations[idxMse];
	MeasureOperation* opPsnr = operations[idxPsnr];
	for (int i=0; i<PLANE_COUNT; i++)
	{
		double mse = 0;
		if (i<PLANE_COLOR)
		{
			mse = ComputeMSE(source1, source2, FramePtr(), i);
		}else
		{
			mse = (opMse->results[PLANE_Y]*4 + opMse->results[PLANE_U] + opMse->results[PLANE_V])/6;
		}

		double mse_min = qMax(mse, 0.001);
		opMse->results[i] = mse;
		opPsnr->results[i] = 20.0*log10(255.0) - 10.0*log10(mse_min);
	}
}


/*
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
*/