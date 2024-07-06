/*=============================================================================
    Name    : d3dblt.cpp
    Purpose : Direct3D texture blitters

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"
#include "d3dblt.h"

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_COLORINDEX
    Description : copies a GL colorindexed image to a D3D surface
    Inputs      : surf - the surface to blit onto
                  data - the GL texture to blit from
                  width, height - dimensions
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_blt_COLORINDEX(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    RETrackFunction();

    D3DLOCKED_RECT lockedRect;
    HRESULT hr;

    hr = surf->LockRect(&lockedRect, NULL, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_COLORINDEX: LockRect failed", hr);
        return GL_FALSE;
    }

    BYTE* psurfBase = (BYTE*)lockedRect.pBits;
    GLint pitch = lockedRect.Pitch;

    if (pitch == width)
    {
        memcpy(psurfBase, data, width * height);
    }
    else
    {
        for (GLint y = 0; y < height; y++)
        {
            memcpy(psurfBase + y * pitch, data + y * width, width);
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_setup
    Description :
    Inputs      : ddpf - pixelformat of the target surface
                  dColorBits - [r,g,b,a] number of bits per gun
                  dColorShift - [r,g,b,a] number of bits to shift into pos
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
static void d3d_blt_setup(D3DFORMAT format, GLint dColorBits[], GLint dColorShift[])
{
    // ASSERT_UNTESTED();

    switch (format)
    {
        case D3DFMT_A8R8G8B8:
            dColorBits[0] = 8; // R
            dColorBits[1] = 8; // G
            dColorBits[2] = 8; // B
            dColorBits[3] = 8; // A
            dColorShift[0] = 16; // R
            dColorShift[1] = 8;  // G
            dColorShift[2] = 0;  // B
            dColorShift[3] = 24; // A
            break;
        case D3DFMT_X8R8G8B8:
            dColorBits[0] = 8; // R
            dColorBits[1] = 8; // G
            dColorBits[2] = 8; // B
            dColorBits[3] = 0; // A (no alpha)
            dColorShift[0] = 16; // R
            dColorShift[1] = 8;  // G
            dColorShift[2] = 0;  // B
            dColorShift[3] = 0;  // A (no alpha)
            break;
        case D3DFMT_R5G6B5:
            dColorBits[0] = 5; // R
            dColorBits[1] = 6; // G
            dColorBits[2] = 5; // B
            dColorBits[3] = 0; // A (no alpha)
            dColorShift[0] = 11; // R
            dColorShift[1] = 5;  // G
            dColorShift[2] = 0;  // B
            dColorShift[3] = 0;  // A (no alpha)
            break;
        default:
            assert(false && "unimplemented");
            memset(dColorBits, 0, sizeof(dColorBits));
            memset(dColorShift, 0, sizeof(dColorShift));
            break;
    }
}

static int d3d_format_bit_count(D3DFORMAT format)
{
    switch (format)
    {
        case D3DFMT_R8G8B8:
            return 24;
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
            return 32;
        case D3DFMT_R5G6B5:
        case D3DFMT_X1R5G5B5:
        case D3DFMT_A1R5G5B5:
            return 16;
        case D3DFMT_A2B10G10R10:
            return 32;
        default:
            assert(false && "unhandled D3DFORMAT");

            return 0; // Unknown format
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_RGBA_generic
    Description : copies a GL RGBA image to a D3D surface
    Inputs      : surf - the surface to blit onto
                  data - the GL texture to blit from
                  width, height - dimensions
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_blt_RGBA_generic(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    RETrackFunction();

    GLint i, bytesPerPixel, sPos, dPos;
    GLint dColor[4], sColor[4];
    GLint dColorBits[4];
    GLint dColorShift[4];
    GLushort* dShort;
    GLuint* dLong;

    BYTE* psurfBase;
    GLint x, y, pitch;

    HRESULT hr;
    D3DSURFACE_DESC ddsd;

    D3DLOCKED_RECT lockedRect;

    hr = surf->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA_generic: LockRect failed", hr);
        return GL_FALSE;
    }

    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    hr = surf->GetDesc(&ddsd);
    if (FAILED(hr))
	{
        errLog("d3d_blt_RGBA_generic: GetDesc failed", hr);
		return GL_FALSE;
	}

    bytesPerPixel = d3d_format_bit_count(ddsd.Format);
    
    d3d_blt_setup(ddsd.Format, dColorBits, dColorShift);

    for (y = 0; y < height; y++)
    {
        dPos = y * pitch;
        sPos = 4 * y * width;

        for (x = 0; x < width; x++, dPos += bytesPerPixel, sPos += 4)
        {
            sColor[0] = data[sPos + 0]; //red
            sColor[1] = data[sPos + 1]; //green
            sColor[2] = data[sPos + 2]; //blue
            sColor[3] = data[sPos + 3]; //alpha

            for (i = 0; i < 4; i++)
            {
                if (dColorBits[i] == 8)
                {
                    dColor[i] = sColor[i];
                }
                else if (dColorBits[i] < 8)
                {
                    dColor[i] = sColor[i] >> (8 - dColorBits[i]);
                }
                else
                {
                    dColor[i] = sColor[i] << (dColorBits[i] - 8);
                }
            }

            switch (bytesPerPixel)
            {
            case 2:
                dShort = (GLushort*)(psurfBase + dPos);
                *dShort = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                break;

            case 4:
                dLong = (GLuint*)(psurfBase + dPos);
                *dLong = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                       | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                break;

            default:
                ;//FIXME: do something about this error
            }
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_RGBA16_generic
    Description : copies a GL RGBA16 image to a D3D surface
    Inputs      : surf - the surface to blit onto
                  data - the GL texture to blit from
                  width, height - dimensions
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
GLboolean d3d_blt_RGBA16_generic(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    RETrackFunction();

    GLint i, bytesPerPixel, sPos, dPos;
    GLint dColor[4], sColor[4];
    GLint dColorBits[4];
    GLint dColorShift[4];
    GLushort* dShort;
    GLushort* sShort;
    GLuint* dLong;

    BYTE* psurfBase;
    GLint x, y, pitch;

    HRESULT hr;
    D3DSURFACE_DESC ddsd;

    D3DLOCKED_RECT lockedRect;

    hr = surf->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA16_generic: LockRect failed", hr);
        return GL_FALSE;
    }

    hr = surf->GetDesc(&ddsd);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA16_generic: GetDesc failed", hr);
        return GL_FALSE;
    }

    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    bytesPerPixel = d3d_format_bit_count(ddsd.Format);

    d3d_blt_setup(ddsd.Format, dColorBits, dColorShift);

    for (y = 0; y < height; y++)
    {
        dPos = y * pitch;
        sPos = 2 * y * width;

        sShort = (GLushort*)(data + sPos);

        for (x = 0; x < width; x++, dPos += bytesPerPixel, sShort++)
        {
            sColor[0] = ((*sShort) & 0x0F00) >> 8;  //red
            sColor[1] = ((*sShort) & 0x00F0) >> 4;  //green
            sColor[2] =  (*sShort) & 0x000F;        //blue
            sColor[3] = ((*sShort) & 0xF000) >> 12; //alpha

            for (i = 0; i < 4; i++)
            {
                if (dColorBits[i] == 4)
                {
                    dColor[i] = sColor[i];
                }
                else if (dColorBits[i] < 4)
                {
                    dColor[i] = sColor[i] >> (4 - dColorBits[i]);
                }
                else
                {
                    dColor[i] = sColor[i] << (dColorBits[i] - 4);
                }
            }

            switch (bytesPerPixel)
            {
            case 2:
                dShort = (GLushort*)(psurfBase + dPos);
                *dShort = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                break;

            case 4:
                dLong = (GLuint*)(psurfBase + dPos);
                *dLong = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                       | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                break;

            default:
                ;//FIXME: do something about this error
            }
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA_0565(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    RETrackFunction();

    BYTE* psurfBase;
    WORD* psurf;
    GLint x, y, pitch;
    GLubyte* dp;

    HRESULT hr;

    D3DLOCKED_RECT lockedRect;

    hr = surf->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA_0565: LockRect failed", hr);
        return GL_FALSE;
    }

    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    for (y = 0, dp = data; y < height; y++)
    {
        psurf = (WORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp += 4, psurf++)
        {
            *psurf = FORM_RGB565(dp[0],dp[1],dp[2]);
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA_0555(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    RETrackFunction();

    BYTE* psurfBase;
    WORD* psurf;
    GLint x, y, pitch;
    GLubyte* dp;

    HRESULT hr;

    D3DLOCKED_RECT lockedRect;

    hr = surf->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA_0555: LockRect failed", hr);
        return GL_FALSE;
    }

    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    for (y = 0, dp = data; y < height; y++)
    {
        psurf = (WORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp += 4, psurf++)
        {
            *psurf = FORM_RGB555(dp[0],dp[1],dp[2]);
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA_8888(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    RETrackFunction();

    BYTE* psurfBase;
    DWORD* psurf;
    GLint x, y, pitch;
    GLubyte* dp;

    HRESULT hr;

    D3DLOCKED_RECT lockedRect;

    hr = surf->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA_8888: LockRect failed", hr);
        return GL_FALSE;
    }

    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    for (y = 0, dp = data; y < height; y++)
    {
        psurf = (DWORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp += 4, psurf++)
        {
            *psurf = RGBA_MAKE(dp[0],dp[1],dp[2],dp[3]);
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA_4444(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    BYTE* psurfBase;
    WORD* psurf;
    GLint x, y, pitch;
    GLubyte* dp;

    HRESULT hr;

    D3DLOCKED_RECT lockedRect;
    hr = surf->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA_4444: LockRect failed", hr);
        return GL_FALSE;
    }

    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    for (y = 0, dp = data; y < height; y++)
    {
        psurf = (WORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp += 4, psurf++)
        {
            *psurf = ((dp[3] & 0xF0) << 8) |    //a
                     ((dp[0] & 0xF0) << 4) |    //r
                      (dp[1] & 0xF0) |          //g
                     ((dp[2] & 0xF0) >> 4);     //b
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA16_4444(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    RETrackFunction();

    BYTE* psurfBase;
    GLint y, pitch;

    HRESULT hr;

    D3DLOCKED_RECT lockedRect;
    hr = surf->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA16_4444: LockRect failed", hr);
        return GL_FALSE;
    }

    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    if (pitch == 2*width)
    {
        MEMCPY(psurfBase, data, 2 * width * height);
    }
    else
    {
        for (y = 0; y < height; y++)
        {
            MEMCPY(psurfBase + y*pitch, data + y*2*width, 2 * width);
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

//special-case blitter
GLboolean d3d_blt_RGBA16_8888(
    IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height)
{
    RETrackFunction();

    BYTE* psurfBase;
    DWORD* psurf;
    GLint x, y, pitch;
    GLushort* dp;
    WORD r, g, b, a;

    HRESULT hr;

    D3DLOCKED_RECT lockedRect;
    hr = surf->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_blt_RGBA16_8888: LockRect failed", hr);
        return GL_FALSE;
    }

    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    for (y = 0, dp = (GLushort*)data; y < height; y++)
    {
        psurf = (DWORD*)(psurfBase + y*pitch);
        for (x = 0; x < width; x++, dp++, psurf++)
        {
            a = ((*dp) & 0xF000) >> 12;
            r = ((*dp) & 0x0F00) >> 8;
            g = ((*dp) & 0x00F0) >> 4;
            b =  (*dp) & 0x000F;
            a <<= 4;
            r <<= 4;
            g <<= 4;
            b <<= 4;
            *psurf = RGBA_MAKE(r, g, b, a);
        }
    }

    surf->UnlockRect();

    return GL_TRUE;
}

void d3d_draw_quad(GLint xOfs, GLint yOfs, GLsizei width, GLsizei height, ComPtr<IDirect3DTexture9> offscreenTexture)
{
    struct Vertex
    {
        float x, y, z, rhw;
        float u, v;
    };

    Vertex vertices[] =
    {
        {(float)xOfs, (float)yOfs, 0.0f, 1.0f, 0.0f, 1.0f},
        {(float)width + xOfs, (float)yOfs, 0.0f, 1.0f, 1.0f, 1.0f},
        {(float)xOfs, (float)height + yOfs, 0.0f, 1.0f, 0.0f, 0.0f},
        {(float)width + xOfs, (float)height + yOfs, 0.0f, 1.0f, 1.0f, 0.0f}
    };

    HRESULT hr;
    CheckHresult(D3D->d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1));
    CheckHresult(D3D->d3dDevice->SetTexture(0, offscreenTexture.Get()));

    D3D->d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    D3D->d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    D3D->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    D3D->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    D3D->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

    D3D->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

    CheckHresult(D3D->d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex)));
    CheckHresult(D3D->d3dDevice->SetTexture(0, nullptr));

    D3D->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
}

void d3d_draw_pixels_RGBA_generic(
    GLint xOfs, GLint yOfs, GLsizei width, GLsizei height, GLubyte* data)
{
    RETrackFunction();

    constexpr int BytesPerPixel = 4;

    assert(xOfs >= 0 && yOfs >= 0);
    if (xOfs < 0 || yOfs < 0)
    {
        return;
    }

    GLuint ymax = CTX->Buffer.Height - 1;

    ComPtr<IDirect3DSurface9> offscreenSurface;
    D3D->d3dDevice->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, offscreenSurface.GetAddressOf(), nullptr);

    HRESULT hr;

    D3DLOCKED_RECT lockedRect;
    CheckHresult(offscreenSurface->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD));

    BYTE* psurfBase = (BYTE*)lockedRect.pBits;
    int pitch = lockedRect.Pitch;

    yOfs = ymax - yOfs - height;
    GLboolean blended = (CTX->Blend || CTX->AlphaTest) != GL_FALSE;

    for (GLint y = 0; y < height; y++)
    {
        auto sPos = BytesPerPixel * width * y;
        auto sp = data + sPos;

        GLuint* dLong = (GLuint*)(psurfBase + y * pitch);

        if (!blended)
        {
            for (GLsizei x = 0; x < width; x++)
            {
                dLong[x] = RGBA_MAKE(sp[BytesPerPixel * x + 0], sp[BytesPerPixel * x + 1], sp[BytesPerPixel * x + 2], 255);
            }
        }
        else
        {
            for (GLsizei x = 0; x < width; x++)
            {
                if (sp[4 * x + 3] != 0)
                {
                    dLong[x] = RGBA_MAKE(sp[BytesPerPixel * x + 0], sp[BytesPerPixel * x + 1], sp[BytesPerPixel * x + 2], 255);
                }
            }
        }
    }

    CheckHresult(offscreenSurface->UnlockRect());

    // Create a texture from the offscreen surface and copy the surface's data to it
    ComPtr<IDirect3DTexture9> offscreenTexture;
    CheckHresult(D3D->d3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, offscreenTexture.GetAddressOf(), nullptr));

    ComPtr<IDirect3DSurface9> pTextureSurface = nullptr;
    CheckHresult(offscreenTexture->GetSurfaceLevel(0, pTextureSurface.GetAddressOf()));
    CheckHresult(D3D->d3dDevice->UpdateSurface(offscreenSurface.Get(), nullptr, pTextureSurface.Get(), nullptr));

    // Draw the texture
    d3d_draw_quad(xOfs, yOfs, width, height, offscreenTexture);
}

/*-----------------------------------------------------------------------------
    Name        : d3d_draw_pixels_RGBA_pitched
    Description : copies a region from an RGBA framebuffer to the actual (generic) framebuffer
    Inputs      : x0, y0 - location to draw from, to
                  x1, y1 - lower right of rectangle to draw
                  swidth, sheight, spitch - dimensions of framebuffer
                  data - the RGBA data of the entire framebuffer
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void d3d_draw_pixels_RGBA_pitched(GLint x0, GLint y0, GLint x1, GLint y1,
                                  GLsizei swidth, GLsizei sheight, GLsizei spitch,
                                  GLubyte* data)
{
    RETrackFunction();

    GLint i, bytesPerPixel, sPos, dPos;
    GLint width, height;
    GLint dColor[4], sColor[4];
    GLint dColorBits[4];
    GLint dColorShift[4];
    GLushort* dShort;
    GLuint* dLong;
    GLubyte* sp;

    BYTE* psurfBase;
    GLint x, y, pitch;

    HRESULT hr;
    D3DSURFACE_DESC ddsd;
    D3DFORMAT ddpf;

    width  = x1 - x0;
    height = y1 - y0;

    //early exit conditions
    if (x0 >= swidth) return;
    if (x1 < 0) return;
    if (y0 >= sheight) return;
    if (y1 < 0) return;

    //clip left
    while (x0 < 0)
    {
        x0++;
        width--;
    }
    if (x0 >= x1)
    {
        return;
    }

    //clip right
    while ((x0+width) > swidth)
    {
        width--;
    }

    //clip top
    while (y0 < 0)
    {
        y0++;
        height--;
    }
    if (y0 >= y1)
    {
        return;
    }

    //clip bottom
    while ((y0+height) > sheight)
    {
        height--;
    }

    ZeroMemory(&ddsd, sizeof(ddsd));
    D3DLOCKED_RECT lockedRect;
    hr = D3D->BackSurface->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        errLog("d3d_draw_pixels_RGBA_pitched: LockRect failed", hr);
        return;
    }

    hr = D3D->BackSurface->GetDesc(&ddsd);
    if (FAILED(hr))
    {
        errLog("d3d_draw_pixels_RGBA_pitched: GetDesc failed", hr);
        return;
	}

    ddpf = ddsd.Format;
    psurfBase = (BYTE*)lockedRect.pBits;
    pitch = lockedRect.Pitch;

    bytesPerPixel = d3d_format_bit_count(ddpf) >> 3;

    d3d_blt_setup(ddpf, dColorBits, dColorShift);

    for (y = 0; y < height; y++)
    {
        dPos = pitch * (y + y0);
        dPos += bytesPerPixel * x0;

        sPos = spitch * ((sheight - 1) - (y + y0));
        sPos += 4 * x0;
        sp = data + sPos;

#if 0
        if (dColorBits[0] == 8 && dColorBits[1] == 8 && dColorBits[2] == 8 && bytesPerPixel == 4)
        {
            //special-case 32bit
            dLong = (GLuint*)(psurfBase + dPos);
            for (x = 0; x < width; x++)
            {
                dLong[x] = RGBA_MAKE(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2], 255);
            }
        }
        else if (dColorBits[0] == 5 && dColorBits[1] == 6 && dColorBits[2] == 5 && bytesPerPixel == 2)
        {
            //special-case 16bit (565)
            dShort = (GLushort*)(psurfBase + dPos);
            for (x = 0; x < width; x++)
            {
                dShort[x] = FORM_RGB565(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2]);
            }
        }
        else if (dColorBits[0] == 5 && dColorBits[1] == 5 && dColorBits[2] == 5 && bytesPerPixel == 2)
        {
            //special-case 16bit (555)
            dShort = (GLushort*)(psurfBase + dPos);
            for (x = 0; x < width; x++)
            {
                dShort[x] = FORM_RGB555(sp[4*x + 0], sp[4*x + 1], sp[4*x + 2]);
            }
        }
        else
#endif
        {
            for (x = 0; x < width; x++, dPos += bytesPerPixel, sPos += 4)
            {
                for (i = 0; i < 4; i++)
                {
                    sColor[i] = data[sPos + i];
                }

                for (i = 0; i < 4; i++)
                {
                    if (dColorBits[i] == 8)
                    {
                        dColor[i] = sColor[i];
                    }
                    else if (dColorBits[i] < 8)
                    {
                        dColor[i] = sColor[i] >> (8 - dColorBits[i]);
                    }
                    else
                    {
                        dColor[i] = sColor[i] << (dColorBits[i] - 8);
                    }
                }

                switch (bytesPerPixel)
                {
                case 2:
                    dShort = (GLushort*)(psurfBase + dPos);
                    *dShort = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                    break;

                case 4:
                    dLong = (GLuint*)(psurfBase + dPos);
                    *dLong = (dColor[0] << dColorShift[0]) | (dColor[1] << dColorShift[1])
                        | (dColor[2] << dColorShift[2]) | (dColor[3] << dColorShift[3]);
                    break;

                default:
                    ;//FIXME: do something about this error
                }
            }
        }
    }

    D3D->BackSurface->UnlockRect();
}
