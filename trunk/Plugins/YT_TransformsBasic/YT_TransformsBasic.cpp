#include "YT_TransformsBasicPlugin.h"
#include "YT_TransformsBasic.h"

ShowYUVComponent::ShowYUVComponent() : m_EmptyFrame(GetHost()->NewFrame())
{
}
ShowYUVComponent::~ShowYUVComponent()
{
	m_EmptyFrame.clear();
}


void ShowYUVComponent::ReleaseBuffers()
{
	
}

RESULT ShowYUVComponent::GetSupportedModes( FormatPtr sourceFormat, QList<QString>& outputNames, QList<QString>& statNames )
{
	outputNames.clear();
	statNames.clear();

	switch (sourceFormat->Color())
	{
	case I420:
	case IYUV:
	case YV12:
		outputNames.append("Y Component");
		outputNames.append("U Component");
		outputNames.append("V Component");
		break;
	case NV12:
		outputNames.append("Y Component");
		break;
	}
	
	return OK;
}

RESULT ShowYUVComponent::Process(const FramePtr input, QMap<QString, FramePtr>& outputs, QMap<QString, QVariant>& stats)
{
	if (input->Format() != m_EmptyFrame->Format())
	{
		m_EmptyFrame->SetFormat(input->Format());
		m_EmptyFrame->Allocate();

		memset(m_EmptyFrame->Data(0), 128, m_EmptyFrame->Format()->PlaneSize(0));
		memset(m_EmptyFrame->Data(1), 128, m_EmptyFrame->Format()->PlaneSize(1));
		memset(m_EmptyFrame->Data(2), 128, m_EmptyFrame->Format()->PlaneSize(2));
		memset(m_EmptyFrame->Data(3), 128, m_EmptyFrame->Format()->PlaneSize(3));
	}

	stats.clear();

	QMapIterator<QString, FramePtr> itr(outputs);
	while (itr.hasNext()) 
	{
		itr.next();
		const QString& name = itr.key();
		FramePtr output = itr.value();
		QString component = QString(name.at(0));

		for (int j=0; j<4; j++)
		{
			if (component.compare(input->Format()->PlaneName(j)) == 0)
			{
				ProcessPlane(input, output, j);
				break;
			}
		}
	}

	return OK;
}

void ShowYUVComponent::Process( FramePtr source1, FramePtr source2, QList<TransformOperation>& operations )
{
	for (int plane=0; plane<4; plane++)
	{
		for (int j=0; j<operations.size(); j++)
		{
			TransformOperation& t = operations[j];
			if (t.plane == plane)
			{
				ProcessPlane(source1, t.frameResult, plane);
			}
		}
	}
}

void ShowYUVComponent::ProcessPlane( FramePtr input, FramePtr output, int plane )
{
	memcpy(output->Data(0), input->Data(plane), input->Format()->PlaneSize(0));
	memset(output->Data(1), 128, output->Format()->PlaneSize(1));
	memset(output->Data(2), 128, output->Format()->PlaneSize(2));
}

RESULT ShowYUVComponent::GetFormat( FormatPtr sourceFormat, const QString& outputName, FormatPtr outputFormat )
{
	QString component = QString(outputName.at(0));

	for (int j=0; j<4; j++)
	{
		if (component.compare(sourceFormat->PlaneName(j)) == 0)
		{
			outputFormat->SetColor(I420);
			outputFormat->SetStride(0, 0);
			outputFormat->SetWidth(sourceFormat->PlaneWidth(j));
			outputFormat->SetHeight(sourceFormat->PlaneHeight(j));
			return OK;
		}
	}
	return E_UNKNOWN;
}

RESULT ShowYUVComponent::GetFormat( unsigned int transformId, int plane, FormatPtr sourceFormat, FormatPtr outputFormat )
{
	if (plane>=0 && plane<=2)
	{
		outputFormat->SetColor(I420);
		outputFormat->SetStride(0, 0);
		outputFormat->SetWidth(sourceFormat->PlaneWidth(plane));
		outputFormat->SetHeight(sourceFormat->PlaneHeight(plane));
		return OK;
	}
	return E_UNKNOWN;
}

const QList<TransformCapabilities>& ShowYUVComponent::GetCapabilities()
{
	if (m_Capabilities.isEmpty())
	{
		TransformCapabilities cap;
		memset(&cap, 0, sizeof(cap));

		cap.transformId = TRANSFORM_SHOW_PLANE;
		cap.outputName = "Show Plane";
		
		cap.inputColors[cap.inputColorsCount++] = I420;
		cap.inputColors[cap.inputColorsCount++] = YV12;

		cap.supportPlanes = true;
		cap.outputFrame = true;

		m_Capabilities.append(cap);
	}

	return m_Capabilities;
}
