#include "YT_TransformsBasicPlugin.h"
#include "YT_TransformsBasic.h"

Q_EXPORT_PLUGIN2(TransformsBasic, TransformsBasicPlugin)

Host* g_Host = 0;
Host* GetHost()
{
	return g_Host;
}

RESULT TransformsBasicPlugin::Init( Host* host)
{
	g_Host = host;

	g_Host->RegisterPlugin(this, PLUGIN_TRANSFORM, QString("Show YUV Plane"));

	return OK;
}

Transform* TransformsBasicPlugin::NewTransform( const QString& name )
{
	return new ShowYUVComponent;	
}

void TransformsBasicPlugin::ReleaseTransform( Transform* t)
{
	delete t;
}
