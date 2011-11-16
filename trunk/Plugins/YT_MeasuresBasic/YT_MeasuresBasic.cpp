#include "YT_MeasuresBasicPlugin.h"
#include "YT_MeasuresBasic.h"
#include "color_map.h"

MeasuresBasic::MeasuresBasic()
{
}

MeasuresBasic::~MeasuresBasic()
{
}

double MeasuresBasic::ComputeMSE( FramePtr input1, FramePtr input2, int plane, DistMapPtr mseMap)
{
	int width = input1->Format()->PlaneWidth(plane);
	int height = input1->Format()->PlaneHeight(plane);

	if (mseMap)
	{
		if (mseMap->size() < width*height)
		{
			mseMap->resize(width*height);
		}
	}

	unsigned char* p1 = input1->Data(plane);
	unsigned char* p2 = input2->Data(plane);
	int stride1 = input1->Format()->Stride(plane);
	int stride2 = input2->Format()->Stride(plane);

	double mse = 0;
	int mseI = 0;
	for (int i=0; i<height; i++)
	{
		for (int j=0; j<width; j++)
		{
			int diff = ((int)p1[j])-((int)p2[j]);
			diff = diff*diff;
			mse += diff;

			if (mseMap)
			{
				(*mseMap)[mseI++] = (double)diff;
			}
		}
		p1 += stride1;
		p2 += stride2;
	}
	mse /= width;
	mse /= height;

	return mse;
}

const MeasureCapabilities& MeasuresBasic::GetCapabilities()
{
	if (m_Capabilities.measures.size() == 0)
	{
		m_Capabilities.hasColorDistortionMap = false;
		m_Capabilities.hasPlaneDistortionMap = true;

		MeasureInfo info = {0};
		info.name = "MSE";
		info.unit = "";
		info.lowerRange = 1;
		info.upperRange = 400;
		info.biggerValueIsBetter = false;
		m_Capabilities.measures.append(info);

		info.name = "PSNR";
		info.unit = "dB";
		info.lowerRange = 22;
		info.upperRange = 50;
		info.biggerValueIsBetter = true;
		m_Capabilities.measures.append(info);
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

	MeasureOperation* opMse = (idxMse>=0)?operations[idxMse]:0;
	MeasureOperation* opPsnr = (idxPsnr>=0)?operations[idxPsnr]:0;
	DistMapPtr mseMap;
	int distMapWidth = 0;
	int distMapHeight = 0;
	
	if (opMse)
	{
		opMse->results[PLANE_Y] = opMse->results[PLANE_U] = 
			opMse->results[PLANE_V] = opMse->results[PLANE_COLOR] = 0;
	}
	
	if (opPsnr)
	{
		opPsnr->results[PLANE_Y] = opPsnr->results[PLANE_U] = 
			opPsnr->results[PLANE_V] = opPsnr->results[PLANE_COLOR] = 0;
	}

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

				double mse = 0;
				if (i == plane)
				{
					if (opMse)
					{
						mseMap = opMse->distMap;
					}else if (opPsnr)
					{
						mseMap = opPsnr->distMap;
					}
					
					if (mseMap)
					{
						distMapWidth = width1;
						distMapHeight = height1;
					}

					mse = ComputeMSE(source1, source2, i, mseMap);
				}else
				{
					mse = ComputeMSE(source1, source2, i, DistMapPtr());
				}

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

	if (mseMap)
	{
		int mapSize = distMapWidth*distMapHeight;
		if (opPsnr && opPsnr->distMap)
		{
			if (opMse && opMse->distMap)
			{
				// generate PSNR map from MSE map
				if (opPsnr->distMap->size()<mapSize)
				{
					opPsnr->distMap->resize(mapSize);
				}

				double c = 20.0*log10(255.0);
				for (int i=0; i<mapSize; i++)
				{
					(*opPsnr->distMap)[i] = c - 10.0*log10((*opMse->distMap)[i]);
				}
			}else
			{
				// generate PSNR map in-place
				double c = 20.0*log10(255.0);
				for (int i=0; i<mapSize; i++)
				{
					(*opPsnr->distMap)[i] = c - 10.0*log10((*opPsnr->distMap)[i]);
				}
			}

			opPsnr->distMapWidth = distMapWidth;
			opPsnr->distMapHeight = distMapHeight;
		}

		if (opMse)
		{
			opMse->distMapWidth = distMapWidth;
			opMse->distMapHeight = distMapHeight;
		}
	}
}
