/*=============================================================================
    Name    : d3dinit.cpp
    Purpose : Direct3D initialization

    Created 10/3/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"
#include "d3dlist.h"
#include "d3denum.h"
#include "d3dtex.h"
#include "d3dinit.h"

#include "3dhw.h"

#include <assert.h>

//initialize game window
void init_window(d3d_context* d3d)
{
    enum_dd_devices(d3d);

    d3d->canRenderWindowed = GL_TRUE;

    d3d->canGammaControl = GL_TRUE;
    d3d->hwnd = (HWND)rglCreate0();

    GetClientRect(d3d->hwnd, &d3d->ScreenRect);
    GetClientRect(d3d->hwnd, &d3d->ViewportRect);
    ClientToScreen(d3d->hwnd, (POINT*)&d3d->ScreenRect.left);
    ClientToScreen(d3d->hwnd, (POINT*)&d3d->ScreenRect.right);
}

GLboolean init_depthsurface(d3d_context* d3d, D3DFORMAT depthFormat)
{
    HRESULT hr = d3d->d3dDevice->GetDepthStencilSurface(&d3d->DepthSurface);

    if (FAILED(hr))
    {
        errLog("init_depthsurface(GetDepthStencilSurface)", hr);
        return GL_FALSE;
    }

    return GL_TRUE;
}

GLboolean init_depthbuffer(d3d_context* d3d)
{
    return init_depthsurface(d3d, D3DFMT_D24X8);
}

//power of 2 <= n
GLint MIN_P2(GLint n)
{
    if (n == 0) return 0;
    if (n < 4) return 2;
    if (n < 8) return 4;
    if (n < 16) return 8;
    if (n < 32) return 16;
    if (n < 64) return 32;
    if (n < 128) return 64;
    if (n < 256) return 128;
    if (n < 512) return 256;
    if (n < 1024) return 512;
    //return 1024;

    return 4096;
}

//power of 2 >= n
GLint MAX_P2(GLint n)
{
    if (n < 2) return n;
    if (n <= 2) return 2;
    if (n <= 4) return 4;
    if (n <= 8) return 8;
    if (n <= 16) return 16;
    if (n <= 32) return 32;
    if (n <= 64) return 64;
    if (n <= 128) return 128;
    if (n <= 256) return 256;
    if (n <= 512) return 512;
    return 1024;
}

static void log_device_caps(d3d_context* d3d)
{
    spdlog::info("canRenderWindowed {}", d3d->canRenderWindowed);
    spdlog::info("canGammaControl {}", d3d->canGammaControl);
    spdlog::info("maxTexAspect {}", d3d->maxTexAspect);
    spdlog::info("minTexWidth {}", d3d->minTexWidth);
    spdlog::info("maxTexWidth {}", d3d->maxTexWidth);
    spdlog::info("minTexHeight {}", d3d->minTexHeight);
    spdlog::info("maxTexHeight {}", d3d->maxTexHeight);
    spdlog::info("canTexModulate {}", d3d->canTexModulate);
    spdlog::info("canTexSelectArg1 {}", d3d->canTexSelectArg1);
    spdlog::info("canTexAdd {}", d3d->canTexAdd);
    spdlog::info("canZCmpLess {}", d3d->canZCmpLess);
    spdlog::info("canZCmpLessEqual {}", d3d->canZCmpLessEqual);
    spdlog::info("canSrcBlendSrcAlpha {}", d3d->canSrcBlendSrcAlpha);
    spdlog::info("canSrcBlendOne {}", d3d->canSrcBlendOne);
    spdlog::info("canSrcBlendZero {}", d3d->canSrcBlendZero);
    spdlog::info("canDestBlendInvSrcAlpha {}", d3d->canDestBlendInvSrcAlpha);
    spdlog::info("canDestBlendOne {}", d3d->canDestBlendOne);
    spdlog::info("canDestBlendZero {}", d3d->canDestBlendZero);
    spdlog::info("canAlphaTestGreater {}", d3d->canAlphaTestGreater);
    spdlog::info("canAlphaTestLess {}", d3d->canAlphaTestLess);
    spdlog::info("squareOnly {}", d3d->squareOnly);
    spdlog::info("canFilterLinear {}", d3d->canFilterLinear);
    spdlog::info("canFilterNearest {}", d3d->canFilterNearest);
    spdlog::info("canClamp {}", d3d->canClamp);
    spdlog::info("canWrap {}", d3d->canWrap);
}

GLboolean init_d3d(d3d_context* d3d)
{
    HRESULT hr;

    D3DPRESENT_PARAMETERS d3dpp{};
    d3dpp.Windowed = !d3d->Fullscreen;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8; // 32-bit only
    d3dpp.BackBufferWidth = CTX->Buffer.Width;
    d3dpp.BackBufferHeight = CTX->Buffer.Height;
    d3dpp.BackBufferCount = 1; // TODO D3D9: Configurable triple buffering
    d3dpp.hDeviceWindow = d3d->hwnd;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // TODO D3D9: Configurable vsync
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
    d3dpp.EnableAutoDepthStencil = TRUE;

    // TODO D3D9: Support multiple adapters
    hr = d3d->d3dObject->CreateDevice(
        0,
        D3DDEVTYPE_HAL,
        d3d->hwnd,
        D3DCREATE_PUREDEVICE | 
        D3DCREATE_HARDWARE_VERTEXPROCESSING | 
        D3DCREATE_MULTITHREADED |
        D3DCREATE_FPU_PRESERVE, // The FPU preserve flag here is important: Audio and various other things will break in Cataclysm if D3D changes the FPU precision
        &d3dpp,
        &d3d->d3dDevice);

    if (FAILED(hr))
    {
        errLog("init_d3d(CreateDevice)", hr);
        return GL_FALSE;
    }

    if (!init_depthbuffer(d3d))
    {
        errLog("init_depthbuffer(CreateDevice)", hr);
        return GL_FALSE;
    }

    assert(devList.size() > 0);
    D3DCAPS9& caps = devList.front().caps;

    //texture sizes
    d3d->maxTexAspect = MIN_P2(caps.MaxTextureAspectRatio);
    d3d->minTexWidth  = MAX_P2(0);
    d3d->minTexHeight = MAX_P2(0);
    d3d->maxTexWidth  = MIN_P2(caps.MaxTextureWidth);
    d3d->maxTexHeight = MIN_P2(caps.MaxTextureHeight);

    if (caps.TextureCaps & D3DPTEXTURECAPS_POW2)
	{
        // TODO: If we need to support this, draw_quad needs to be changed to pad up to nearest power of 2
        spdlog::error("Non-power of 2 texture support is required");
        return GL_FALSE;
	}

    //depthbuffer comparison
    d3d->canZCmpLess = caps.ZCmpCaps & D3DPCMPCAPS_LESS; 
    d3d->canZCmpLessEqual = caps.ZCmpCaps & D3DPCMPCAPS_LESSEQUAL;

    //source blending
    d3d->canSrcBlendSrcAlpha = caps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA;
    d3d->canSrcBlendOne = caps.SrcBlendCaps & D3DPBLENDCAPS_ONE;
    d3d->canSrcBlendZero = caps.SrcBlendCaps & D3DPBLENDCAPS_ZERO;
    if (d3d->canSrcBlendSrcAlpha)
    {
        d3d->srcBlendFallback = D3DBLEND_SRCALPHA;
    }
    else if (d3d->canSrcBlendOne)
    {
        d3d->srcBlendFallback = D3DBLEND_ONE;
    }
    else
    {
        d3d->srcBlendFallback = D3DBLEND_ZERO;
    }

    //destination blending
    d3d->canDestBlendInvSrcAlpha = caps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA;
    d3d->canDestBlendOne = caps.DestBlendCaps & D3DPBLENDCAPS_ONE;
    d3d->canDestBlendZero = caps.DestBlendCaps & D3DPBLENDCAPS_ZERO;
    d3d->destBlendFallback = D3DBLEND_INVSRCALPHA;

    //alphatest
    d3d->canAlphaTestGreater = caps.AlphaCmpCaps & D3DPCMPCAPS_GREATER;
    d3d->canAlphaTestLess = caps.AlphaCmpCaps & D3DPCMPCAPS_LESS;
    if (caps.AlphaCmpCaps & D3DPCMPCAPS_NEVER)
    {
        d3d->alphaTestFallback = D3DCMP_NEVER;
    }
    if (caps.AlphaCmpCaps & D3DPCMPCAPS_ALWAYS)
    {
        d3d->alphaTestFallback = D3DCMP_ALWAYS;
    }

    //square-only cap
    d3d->squareOnly = caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY;

    //texture filtering
    d3d->canFilterLinear = TRUE; // required for D3D9 hardware
    d3d->canFilterNearest = TRUE; // required for D3D9 hardware

    //texture address
    d3d->canClamp = caps.TextureAddressCaps & D3DPTADDRESSCAPS_CLAMP;
    d3d->canWrap = caps.TextureAddressCaps & D3DPTADDRESSCAPS_WRAP;

    d3d->canTexModulate = caps.TextureOpCaps & D3DTEXOPCAPS_MODULATE;
    d3d->canTexSelectArg1 = caps.TextureOpCaps & D3DTEXOPCAPS_SELECTARG1;
    d3d->canTexAdd = caps.TextureOpCaps & D3DTEXOPCAPS_ADD;

    //can we alphablend at all ?
    if (d3d->canSrcBlendSrcAlpha || d3d->canSrcBlendOne || d3d->canSrcBlendZero)
    {
        if (d3d->canDestBlendInvSrcAlpha || d3d->canDestBlendOne || d3d->canDestBlendZero)
        {
            d3d->canAlphaBlend = GL_TRUE;
        }
        else
        {
            d3d->canAlphaBlend = GL_FALSE;
        }
    }
    else
    {
        d3d->canAlphaBlend = GL_FALSE;
    }

    //can we alphatest at all ?
    if (d3d->canAlphaTestGreater || d3d->canAlphaTestLess)
    {
        d3d->canAlphaTest = GL_TRUE;
    }
    else
    {
        d3d->canAlphaTest = GL_FALSE;
    }

    log_device_caps(d3d);

    d3d->d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &d3d->BackSurface);

    return GL_TRUE;
}

GLboolean init_viewport(d3d_context* d3d)
{
    D3DVIEWPORT9 view{};

    // Set the viewport dimensions based on the screen rectangle
    view.X = 0;
    view.Y = 0;
    view.Width = d3d->ScreenRect.right - d3d->ScreenRect.left;
    view.Height = d3d->ScreenRect.bottom - d3d->ScreenRect.top;
    view.MinZ = 0.0f;
    view.MaxZ = 1.0f;

    // Set the viewport on the device
    HRESULT hr = d3d->d3dDevice->SetViewport(&view);
    if (FAILED(hr))
    {
        errLog("init_viewport(SetViewport)", hr);
        return GL_FALSE;
    }

    return GL_TRUE;
}

static void LoadIdentity(D3DMATRIX* mat)
{
    mat->_11 = mat->_22 = mat->_33 = mat->_44 = 1.0f;
    mat->_12 = mat->_13 = mat->_14 = mat->_41 = 0.0f;
    mat->_21 = mat->_23 = mat->_24 = mat->_42 = 0.0f;
    mat->_31 = mat->_32 = mat->_34 = mat->_43 = 0.0f;
}

static void init_matrices(d3d_context* d3d)
{
    D3DMATRIX mat;
    D3DMATRIX matWorld;
    D3DMATRIX matView;
    D3DMATRIX matProj;

    LoadIdentity(&mat);

    matWorld = mat;
    d3d->d3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    matView = mat;
    d3d->d3dDevice->SetTransform(D3DTS_VIEW, &matView);

    matProj = mat;
    d3d->d3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

GLboolean initialize_directx(GLcontext* ctx)
{
    d3d_context* d3d = (d3d_context*)ctx->DriverCtx;

    d3d->Fullscreen = rglGetFullscreen();

    init_window(d3d);

    if (!init_d3d(d3d))
    {
        d3d_shutdown(ctx);
        return GL_FALSE;
    }

    enum_texture_formats(d3d, D3DFMT_X8R8G8B8);

    if (!init_viewport(d3d))
    {
        d3d_shutdown(ctx);
        return GL_FALSE;
    }

    init_matrices(d3d);

    return GL_TRUE;
}

void d3d_shutdown(GLcontext* ctx)
{
    d3d_context* d3d = (d3d_context*)ctx->DriverCtx;

    d3d_end_scene(d3d);

    LOG(NULL);

    if (d3d->Fullscreen)
    {
        d3d->d3dDevice->SetGammaRamp(0, D3DSGR_CALIBRATE, &d3d->awOldLUT);
    }

#if _DEBUG
#define ReleaseAndVerify(x) \
    refCount = x.Reset(); \
	if (refCount != 0) \
	{ \
		spdlog::error("Releasing " #x " failed: {} refs remaining", refCount); \
        __debugbreak(); \
	}
#else
#define ReleaseAndVerify(x) \
    refCount = x.Reset(); \
	if (refCount != 0) \
	{ \
		spdlog::error("Releasing " #x " failed: {} refs remaining", refCount); \
	}
#endif

    unsigned long refCount;
    ReleaseAndVerify(d3d->DepthSurface);
    ReleaseAndVerify(d3d->BackSurface);
    ReleaseAndVerify(d3d->d3dDevice);


    // ReleaseAndVerify(d3d->d3dObject);
    // FIXME: One dangling ref here
    d3d->d3dObject.Reset();
}
