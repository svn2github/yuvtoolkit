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


class YTR_D3DPlugin : public QObject, public YT_PlugIn
{
	Q_OBJECT;
	Q_INTERFACES(YT_PlugIn);
public:
	virtual YT_RESULT Init(YT_Host*);

	virtual YT_Renderer* NewRenderer(QWidget* widget, const QString& name);
	virtual void ReleaseRenderer(YT_Renderer*);
};

class YTR_D3D : public D3DWidget, public YT_Renderer
{
	Q_OBJECT
public:
	YTR_D3D(YT_Host* host, QWidget* widget, const QString& name);
	~YTR_D3D();

	virtual QWidget* GetWidget() {return this;}
	virtual YT_RESULT RenderScene(QList<YT_Render_Frame>& frameList);

	// Allocate render specific buffers
	virtual YT_RESULT Allocate(YT_Frame_Ptr& frame, YT_Format_Ptr sourceFormat);
	virtual YT_RESULT Deallocate(YT_Frame_Ptr frame);

	// Prepare YT_Frame_Ptr before using it
	virtual YT_RESULT GetFrame(YT_Frame_Ptr& frame);
	virtual YT_RESULT ReleaseFrame(YT_Frame_Ptr frame);

	virtual bool NeedReset();

	// If any function returns YT_E_RENDER_RESET, caller need to
	// 1. Release all render frames using ReleaseFrame(..)
	// 2. Deallocate all render frames using Deallocate(..)
	// 3. Call Reset()
	// 4. Reallocate frames again using Allocae(..)
	virtual YT_RESULT Reset();

public slots:
	void OnResizeTimer();
protected:
	YT_Host* m_Host;
	bool m_NeedReset;

	int counter;
};

#endif // YTR_D3D_H
