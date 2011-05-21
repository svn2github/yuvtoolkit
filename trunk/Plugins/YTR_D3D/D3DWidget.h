#ifndef D3D_WIDGET_H
#define D3D_WIDGET_H

#include <QtGui>
#include <d3d9.h>

class D3DWidget : public QWidget
{
	Q_OBJECT;
	friend class D3DDevicePtr;
public:
	D3DWidget(QWidget* parent);
	virtual ~D3DWidget();
	
	QPaintEngine *paintEngine() const;
	virtual void resizeEvent ( QResizeEvent* );

	// QMutex* GetLock() {return &mutex;}
	IDirect3DDevice9* GetDevice() {return d3DDevice;}

	IDirect3DSurface9* GetSurface(UINT width, UINT height, D3DFORMAT format);

protected:
	HRESULT InitD3D();
	HRESULT UnInitD3D();
	HRESULT ResetD3D();

	int bufferWidth;
	int bufferHeight;

	IDirect3D9*                  d3D;
	IDirect3DDevice9*            d3DDevice;
	UINT                         adapter;
	UINT                         resetToken;
	D3DPRESENT_PARAMETERS        d3dpp;
};


#endif