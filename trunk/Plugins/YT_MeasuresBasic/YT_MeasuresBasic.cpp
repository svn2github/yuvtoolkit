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

		op->hasResults[PLANE_Y] = 
			op->hasResults[PLANE_U] = 
			op->hasResults[PLANE_V] = 
			op->hasResults[PLANE_COLOR] = 
			false;

		if (op->measureName == "MSE")
		{
			idxMse = i;
		}else if (op->measureName == "PSNR")
		{
			idxPsnr = i;
		}
	}

	MeasureOperation* opMse = operations[idxMse];
	MeasureOperation* opPsnr = operations[idxPsnr];

	opMse->results[PLANE_Y] = opMse->results[PLANE_U] = 
		opMse->results[PLANE_V] = opMse->results[PLANE_COLOR] = 0;
	opPsnr->results[PLANE_Y] = opPsnr->results[PLANE_U] = 
		opPsnr->results[PLANE_V] = opPsnr->results[PLANE_COLOR] = 0;

	int weightSum = 0;
	for (int i=0; i<PLANE_COUNT; i++)
	{
		if (i<PLANE_COLOR)
		{
			int width1 = source1->Format()->PlaneWidth(i);
			int height1 = source1->Format()->PlaneHeight(i);
			int width2 = source2->Format()->PlaneWidth(i);
			int height2 = source2->Format()->PlaneHeight(i);
			
			if (width1 == width2 && height1 == height2 && width1>0 && height1>0)
			{
				int weightPlane = source1->Format()->Width()*source1->Format()->Height()*4/width1/height1;
				weightSum += weightPlane;

				double mse = ComputeMSE(source1, source2, FramePtr(), i);

				opMse->hasResults[i] = true;
				opMse->results[i] = mse;
				opMse->results[PLANE_COLOR] += weightPlane*mse;
			}
		}else
		{
			opMse->hasResults[i] = true;
			opMse->results[i] /= weightSum;
		}

		if (opMse->hasResults[i] && weightSum>0)
		{
			double mse_min = qMax(opMse->results[i], 0.001);
			opPsnr->results[i] = 20.0*log10(255.0) - 10.0*log10(mse_min);
			opPsnr->hasResults[i] = true;
		}
	}
}
