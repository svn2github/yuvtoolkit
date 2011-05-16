#include "D3DWidget.h"
#include <Windows.h>
#include <assert.h>

D3DWidget::D3DWidget( QWidget* parent ) : QWidget(parent), d3D(NULL),
d3DDevice(NULL), resizeTimer(NULL), bufferWidth(0), bufferHeight(0)
{
	// setAttribute(Qt::WA_NoSystemBackground, true);
	// setAttribute(Qt::WA_NativeWindow, true);
	setAttribute(Qt::WA_PaintOnScreen);

	setGeometry(0,0,parent->width(),parent->height());
	show();

	InitD3D();
}


D3DWidget::~D3DWidget()
{
	UnInitD3D();
}

QPaintEngine * D3DWidget::paintEngine() const
{
	return NULL;
}

void D3DWidget::resizeEvent( QResizeEvent* e)
{
}

HRESULT D3DWidget::InitD3D()
{
	HRESULT hr = S_OK;
	adapter = D3DADAPTER_DEFAULT;
	d3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3D)
	{
		return E_FAIL;
		//return;
	}

	D3DDISPLAYMODE d3ddm;
	d3D->GetAdapterDisplayMode(adapter, &d3ddm);

	// set the presentation parameters
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth            = (width()>640)? 0 : 640;
	d3dpp.BackBufferHeight           = (height()>480)? 0 : 480;
	d3dpp.BackBufferCount            = 1;
	d3dpp.BackBufferFormat           = d3ddm.Format;
	d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
	d3dpp.Windowed                   = TRUE;
	d3dpp.EnableAutoDepthStencil     = TRUE;
	d3dpp.AutoDepthStencilFormat     = D3DFMT_D16;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_ONE;
	d3dpp.Flags                      = D3DPRESENTFLAG_VIDEO;
	d3dpp.hDeviceWindow              = winId();


	hr = d3D->CreateDevice(adapter,
		D3DDEVTYPE_HAL, winId(),
		D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
		&d3dpp, &d3DDevice);

	assert(SUCCEEDED(hr));

	bufferWidth = d3dpp.BackBufferWidth;
	bufferHeight = d3dpp.BackBufferHeight;

	return hr;
}

HRESULT D3DWidget::UnInitD3D()
{
	if (d3DDevice)
	{
		d3DDevice->Release();
	}

	if (d3D)
	{
		d3D->Release();
	}

	return S_OK;
}

HRESULT D3DWidget::ResetD3D()
{
	HRESULT hr = S_OK;
	D3DPRESENT_PARAMETERS _d3dpp = d3dpp;

	_d3dpp.BackBufferWidth            = qMax(width(), 10);
	_d3dpp.BackBufferHeight           = qMax(height(), 10);
	hr = d3DDevice->Reset(&_d3dpp);

	if (SUCCEEDED(hr))
	{
		bufferWidth = _d3dpp.BackBufferWidth;
		bufferHeight = _d3dpp.BackBufferHeight;
	}

	return hr;
}

IDirect3DSurface9* D3DWidget::GetSurface(UINT width, UINT height, D3DFORMAT format)
{
	//QMutexLocker locker(&mutex);
	IDirect3DSurface9* surface;

	HRESULT hr = d3DDevice->CreateOffscreenPlainSurface(width, height, format, D3DPOOL_DEFAULT, &surface, NULL);
	assert(SUCCEEDED(hr));

	return surface;
}