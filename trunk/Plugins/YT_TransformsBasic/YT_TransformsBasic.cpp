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

void ShowYUVComponent::ProcessPlane( FramePtr input, FramePtr output, int plane )
{
	output->Reset();
	output->Format()->SetColor(I420);
	output->Format()->SetStride(0, 0);
	output->Format()->SetWidth(input->Format()->PlaneWidth(plane));
	output->Format()->SetHeight(input->Format()->PlaneHeight(plane));
	output->Format()->PlaneSize(0); // Update internal	
	output->SetData(0, input->Data(plane));
	for (int i=1; i<4; i++)
	{
		output->SetData(i, m_EmptyFrame->Data(i));
	}
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
