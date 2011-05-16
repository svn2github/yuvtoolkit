#ifndef YT_TRANSFORMSBASIC_PLUGIN_H
#define YT_TRANSFORMSBASIC_PLUGIN_H

#include "../YT_Interface.h"


class YT_TransformsBasicPlugin : public QObject, public YT_PlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YT_PlugIn);
public:
	virtual YT_RESULT Init(YT_Host*);

	virtual YT_Transform* NewTransform(const QString& name);
	virtual void ReleaseTransform(YT_Transform*);
};

#endif // YT_TRANSFORMSBASIC_PLUGIN_H
