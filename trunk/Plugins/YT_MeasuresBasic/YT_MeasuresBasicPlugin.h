#ifndef YT_MEASURESBASICPLUGIN_H
#define YT_MEASURESBASICPLUGIN_H

#include "../YT_Interface.h"

class YT_MeasuresBasicPlugin : public QObject, public YT_PlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YT_PlugIn);
public:
	virtual YT_RESULT Init(YT_Host*);

	virtual YT_Measure* NewMeasure(const QString& name);
	virtual void ReleaseMeasure(YT_Measure*);
};

#endif // YT_MEASURESBASICPLUGIN_H
