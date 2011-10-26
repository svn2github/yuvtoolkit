#include "YTR_D3D.h"
#include <assert.h>

#define RENDER_FREQ	60

Q_EXPORT_PLUGIN2(YTR_D3D, YTR_D3DPlugin)

Host* g_Host = 0;
Host* GetHost()
{
	return g_Host;
}

RESULT YTR_D3DPlugin::Init( Host* host)
{
	g_Host = host;

	g_Host->RegisterPlugin(this, PLUGIN_RENDERER, QString("D3D"));

	return OK;
}


Renderer* YTR_D3DPlugin::NewRenderer(QWidget* widget, const QString& name )
{
	YTR_D3D* renderer = new YTR_D3D(g_Host, widget, name);

	return renderer;
}

void YTR_D3DPlugin::ReleaseRenderer( Renderer* parent )
{
	delete (YTR_D3D*)parent;
}



YTR_D3D::YTR_D3D(Host* host, QWidget* widget, const QString& name) 
: m_Host(host), D3DWidget(widget), m_NeedReset(false)
{
	QTimer *timer = new QTimer(widget);
	connect(timer, SIGNAL(timeout()), this, SLOT(OnResizeTimer()));

	counter = 0;
	timer->start(300);
}


YTR_D3D::~YTR_D3D()
{
}

void YTR_D3D::OnResizeTimer()
{
	if (width()>640 || height()>480)
	{
		if (width()!=bufferWidth || height()!=bufferHeight)
		{
			m_NeedReset = true;
		}
	}
}

#define RECT_IS_EMPTY(rc) (rc->left==0 && rc->right==0 && rc->top==0 && rc->bottom==0)
#define RECT_WIDTH(rc) (rc->right-rc->left)
#define RECT_HEIGHT(rc) (rc->bottom-rc->top)

RESULT YTR_D3D::RenderScene(const FrameList& frames)
{
	if (FAILED(d3DDevice->TestCooperativeLevel()))
	{
		m_NeedReset = true;
		return E_RENDER_RESET;
	}

	int width = QWidget::width();
	int height = QWidget::height();
	if (width == 0 || height == 0)
	{
		return E_UNKNOWN;
	}

	HRESULT hr = S_OK;
	hr = d3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
	assert(SUCCEEDED(hr));

	IDirect3DSurface9* pRT = 0;
	hr = d3DDevice->GetRenderTarget(0, &pRT);
	assert(SUCCEEDED(hr));

	D3DSURFACE_DESC desc;
	pRT->GetDesc(&desc);
	double scale_x = 1;
	double scale_y = 1;
	if (desc.Width != width || desc.Height != height)
	{
		scale_x = ((double)desc.Width)/width;
		scale_y = ((double)desc.Height)/height;
	}

	for (int i=0; i<frames.size(); ++i) 
	{
		FramePtr frame = frames.at(i);
		if (!frame)
		{
			continue;
		}

		const QRect _srcRect = frame->Info(SRC_RECT).toRect();
		const QRect _dstRect = frame->Info(DST_RECT).toRect();
		IDirect3DSurface9* pSurface = (IDirect3DSurface9*) (frame->ExternData());
		
		RECT srcRectCopy = {_srcRect.left(), _srcRect.top(), _srcRect.right(), _srcRect.bottom()};
		RECT dstRectCopy = {_dstRect.left(), _dstRect.top(), _dstRect.right(), _dstRect.bottom()};

		RECT* srcRect = &(srcRectCopy);
		RECT* dstRect = &(dstRectCopy);

		if (RECT_IS_EMPTY(srcRect))
		{
			srcRect = NULL;
		}

		if (RECT_IS_EMPTY(dstRect))
		{
			dstRect = NULL;
		}else if (scale_x!=1 || scale_y != 1)
		{
			dstRect->left *= scale_x;
			dstRect->right *= scale_x;
			dstRect->top *= scale_y;
			dstRect->bottom *= scale_y;				
		}

		hr = d3DDevice->StretchRect(pSurface, srcRect, 
			pRT, dstRect, D3DTEXF_LINEAR);
	}

	hr = d3DDevice->Present( 0, 0, 0, 0 );
	pRT->Release();

	/*D3DRASTER_STATUS status;
	memset(&status, 0, sizeof(status));
	hr = d3DDevice->GetRasterStatus(0,&status);
	assert(SUCCEEDED(hr));*/

	return OK;
}

RESULT YTR_D3D::Allocate( FramePtr& frame, FormatPtr sourceFormat )
{
	IDirect3DDevice9* d3DDevice = GetDevice();

	if (!d3DDevice || FAILED(d3DDevice->TestCooperativeLevel()))
	{
		m_NeedReset = true;
		return E_UNKNOWN;
	}

	IDirect3DSurface9* surface = NULL;
	HRESULT hr = d3DDevice->CreateOffscreenPlainSurface(sourceFormat->Width(),
		sourceFormat->Height(), D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);
	if (FAILED(hr))
	{
		return E_RENDER_RESET;
	}

	D3DLOCKED_RECT d3d_lr;
	hr = surface->LockRect(&d3d_lr, NULL, D3DLOCK_READONLY);
	if (FAILED(hr))
	{
		return E_RENDER_RESET;
	}
	surface->UnlockRect();
	// D3DSURFACE_DESC d3d_sd;
	// surface->GetDesc(&d3d_sd);

	frame = m_Host->NewFrame();
	frame->SetFormat(sourceFormat);

	frame->SetExternData((void*) surface);
	// support only rgb24
	frame->Format()->SetStride(0, d3d_lr.Pitch);
	frame->Format()->SetStride(1,0);
	frame->Format()->SetStride(2,0);
	frame->Format()->SetStride(3,0);

	frame->Format()->SetColor(XRGB32);
	
	return OK;
}

#define RELEASE_SURFACE(frame) if (frame->ExternData()) {HRESULT hr = ((IDirect3DSurface9*)(frame->ExternData()))->Release(); assert(SUCCEEDED(hr)); frame->SetExternData(NULL);}
RESULT YTR_D3D::Deallocate( FramePtr frame )
{
	RELEASE_SURFACE(frame);

	frame.clear();

	return OK;
}

RESULT YTR_D3D::GetFrame( FramePtr& frame )
{
	D3DLOCKED_RECT d3d_lr;
	IDirect3DSurface9* surface = (IDirect3DSurface9*)(frame->ExternData());
	HRESULT hr = surface->LockRect(&d3d_lr, NULL, NULL);
	assert(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return E_RENDER_RESET;
	}

	frame->SetData(0, (unsigned char*)d3d_lr.pBits);
	frame->SetData(1, 0);
	frame->SetData(2, 0);
	frame->SetData(3, 0);

	counter ++;

	return OK;
}

RESULT YTR_D3D::ReleaseFrame( FramePtr frame )
{
	frame->SetData(0, 0);
	frame->SetData(1, 0);
	frame->SetData(2, 0);
	frame->SetData(3, 0);

	IDirect3DSurface9* surface = (IDirect3DSurface9*)(frame->ExternData());
	
	HRESULT hr = surface->UnlockRect();
	assert(SUCCEEDED(hr));

	counter--;

	return OK;
}

RESULT YTR_D3D::Reset()
{
	HRESULT hr = ResetD3D();	

	if (SUCCEEDED(hr))
	{
		m_NeedReset = false;

		return OK;
	}else
	{
		m_NeedReset = true;

		return E_UNKNOWN;
	}
}

bool YTR_D3D::NeedReset()
{
	return m_NeedReset;
}

