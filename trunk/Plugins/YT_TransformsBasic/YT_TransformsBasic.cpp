#include "YT_TransformsBasic.h"

YT_ShowYUVComponent::YT_ShowYUVComponent() : m_EmptyFrame(GetHost()->NewFrame())
{
}
YT_ShowYUVComponent::~YT_ShowYUVComponent()
{
	m_EmptyFrame.clear();
}


void YT_ShowYUVComponent::ReleaseBuffers()
{
	
}


YT_RESULT YT_ShowYUVComponent::GetSupportedModes( YT_Format_Ptr sourceFormat, QList<QString>& outputNames, QList<QString>& statNames )
{
	outputNames.clear();
	statNames.clear();

	switch (sourceFormat->Color())
	{
	case YT_I420:
	case YT_IYUV:
	case YT_YV12:
		outputNames.append("Y Component");
		outputNames.append("U Component");
		outputNames.append("V Component");
		break;
	case YT_NV12:
		outputNames.append("Y Component");
		break;
	}
	
	return YT_OK;
}

YT_RESULT YT_ShowYUVComponent::Process(const YT_Frame_Ptr input, QMap<QString, YT_Frame_Ptr>& outputs, QMap<QString, QVariant>& stats)
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

	QMapIterator<QString, YT_Frame_Ptr> itr(outputs);
	while (itr.hasNext()) 
	{
		itr.next();
		const QString& name = itr.key();
		YT_Frame_Ptr output = itr.value();
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

	return YT_OK;
}

void YT_ShowYUVComponent::ProcessPlane( YT_Frame_Ptr input, YT_Frame_Ptr output, int plane )
{
	output->Reset();
	output->Format()->SetColor(YT_I420);
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

YT_RESULT YT_ShowYUVComponent::GetFormat( YT_Format_Ptr sourceFormat, const QString& outputName, YT_Format_Ptr outputFormat )
{
	QString component = QString(outputName.at(0));

	for (int j=0; j<4; j++)
	{
		if (component.compare(sourceFormat->PlaneName(j)) == 0)
		{
			outputFormat->SetColor(YT_I420);
			outputFormat->SetStride(0, 0);
			outputFormat->SetWidth(sourceFormat->PlaneWidth(j));
			outputFormat->SetHeight(sourceFormat->PlaneHeight(j));
			return YT_OK;
		}
	}
	return YT_ERROR;
}
