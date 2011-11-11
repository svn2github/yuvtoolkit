#include "YT_MeasuresBasic.h"
#include "YT_MeasuresBasicPlugin.h"

Q_EXPORT_PLUGIN2(MeasuresBasic, MeasuresBasicPlugin)

Host* g_Host = 0;
Host* GetHost()
{
	return g_Host;
}

RESULT MeasuresBasicPlugin::Init( Host* host )
{
	g_Host = host;

	g_Host->RegisterPlugin(this, PLUGIN_MEASURE, QString("Compute PSNR/MSE"));

	return OK;
}

Measure* MeasuresBasicPlugin::NewMeasure( const QString& name )
{
	return new MeasuresBasic;
}

void MeasuresBasicPlugin::ReleaseMeasure( Measure* m )
{
	delete m;
}
