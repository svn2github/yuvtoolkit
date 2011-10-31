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

	g_Host->RegisterPlugin(this, PLUGIN_TRANSFORM, QString("Show Plane"));
	g_Host->RegisterPlugin(this, PLUGIN_TRANSFORM, QString("Measures"));

	return OK;
}

Transform* TransformsBasicPlugin::NewTransform( const QString& name )
{
	if (name == "Show Plane")
	{
		return new ShowYUVComponent;
	}else
	{
		return new MeasuresBasic;
	}
	
}

void TransformsBasicPlugin::ReleaseTransform( Transform* t)
{
	delete t;
}
