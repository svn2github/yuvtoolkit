#include "YT_TransformsBasicPlugin.h"
#include "YT_TransformsBasic.h"
#include "YT_MeasuresBasic.h"

Q_EXPORT_PLUGIN2(TransformsBasic, TransformsBasicPlugin)

Host* g_Host = 0;
Host* GetHost()
{
	return g_Host;
}

RESULT TransformsBasicPlugin::Init( Host* host)
{
	g_Host = host;

	g_Host->RegisterPlugin(this, PLUGIN_MEASURE, QString("Measures"));

	return OK;
}

Measure* TransformsBasicPlugin::NewMeasure( const QString& name )
{
	return new MeasuresBasic;
}

void TransformsBasicPlugin::ReleaseMeasure( Measure* t)
{
	delete t;
}
