#ifndef YTR_D3D_H
#define YTR_D3D_H

#include "../YT_Interface.h"
#include "D3DWidget.h"
#include <d3d9.h>

//HRESULT hr = S_OK;
//D3DDISPLAYMODE mode;
//memset(&mode, 0, sizeof(mode));
//hr = d3DDevice->GetDisplayMode(0,&mode);
//assert(SUCCEEDED(hr));
//
//int refreshRate = mode.RefreshRate/2;


class YTR_D3DPlugin : public QObject, public YTPlugIn
{
	Q_OBJECT;
    Q_INTERFACES(YTPlugIn);
    Q_PLUGIN_METADATA(IID "net.yocto.YUVToolkit.PlugIn/1.3")
public:
	virtual RESULT Init(Host*);

	virtual Renderer* NewRenderer(QWidget* widget, const QString& name);
	virtual void ReleaseRenderer(Renderer*);
};

class YTR_D3D : public D3DWidget, public Renderer
{
	Q_OBJECT
public:
	YTR_D3D(Host* host, QWidget* widget, const QString& name);
	~YTR_D3D();

	virtual QWidget* GetWidget() {return this;}
	virtual RESULT RenderScene(const FrameList& frames);

	// Allocate render specific buffers
	virtual RESULT Allocate(FramePtr& frame, FormatPtr sourceFormat);
	virtual RESULT Deallocate(FramePtr frame);

	// Prepare FramePtr before using it
	virtual RESULT GetFrame(FramePtr& frame);
	virtual RESULT ReleaseFrame(FramePtr frame);

	virtual bool NeedReset();

	// If any function returns E_RENDER_RESET, caller need to
	// 1. Release all render frames using ReleaseFrame(..)
	// 2. Deallocate all render frames using Deallocate(..)
	// 3. Call Reset()
	// 4. Reallocate frames again using Allocae(..)
	virtual RESULT Reset();

public slots:
	void OnResizeTimer();
protected:
	Host* m_Host;
	bool m_NeedReset;

	int counter;
	int m_ResetLastCheckedWidth;
	int m_ResetLastCheckedHeight;
};

#endif // YTR_D3D_H
