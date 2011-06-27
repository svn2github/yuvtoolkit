#include "YT_MeasuresBasic.h"
#include "YT_MeasuresBasicPlugin.h"

Q_EXPORT_PLUGIN2(YT_MeasuresBasic, YT_MeasuresBasicPlugin)

YT_Host* g_Host = 0;
YT_Host* GetHost()
{
	return g_Host;
}

YT_RESULT YT_MeasuresBasicPlugin::Init( YT_Host* host )
{
	g_Host = host;

	g_Host->RegisterPlugin(this, YT_PLUGIN_MEASURE, QString("Compute PSNR/MSE"));

	return YT_OK;
}

YT_Measure* YT_MeasuresBasicPlugin::NewMeasure( const QString& name )
{
	return new YT_MeasuresBasic;
}

void YT_MeasuresBasicPlugin::ReleaseMeasure( YT_Measure* m )
{
	delete m;
}
