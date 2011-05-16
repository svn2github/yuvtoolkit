#include "YT_TransformsBasicPlugin.h"
#include "YT_TransformsBasic.h"

Q_EXPORT_PLUGIN2(YT_TransformsBasic, YT_TransformsBasicPlugin)

YT_Host* g_Host = 0;
YT_Host* GetHost()
{
	return g_Host;
}

YT_RESULT YT_TransformsBasicPlugin::Init( YT_Host* host)
{
	g_Host = host;

	g_Host->RegisterPlugin(this, YT_PLUGIN_TRANSFORM, QString("Show YUV Plane"));

	return YT_OK;
}

YT_Transform* YT_TransformsBasicPlugin::NewTransform( const QString& name )
{
	return new YT_ShowYUVComponent;	
}

void YT_TransformsBasicPlugin::ReleaseTransform( YT_Transform* t)
{
	delete t;
}
