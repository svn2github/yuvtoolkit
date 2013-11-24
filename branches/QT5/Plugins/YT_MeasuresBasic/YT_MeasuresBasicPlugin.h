#ifndef MEASURESBASICPLUGIN_H
#define MEASURESBASICPLUGIN_H

#include "../YT_Interface.h"

class MeasuresBasicPlugin : public QObject, public YTPlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YTPlugIn);
    Q_PLUGIN_METADATA(IID "net.yocto.YUVToolkit.PlugIn/1.3")
public:
	virtual RESULT Init(Host*);

	virtual Measure* NewMeasure(const QString& name);
	virtual void ReleaseMeasure(Measure*);
};

#endif // MEASURESBASICPLUGIN_H
