/*=============================================================================
    Name    : d3drv.h
    Purpose : Direct3D driver for rGL header file

    Created 9/24/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _D3DRV_H
#define _D3DRV_H

#define WIN32_LEAN_AND_MEAN
#define D3D_OVERLOADS

#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include "d3dtypes.h"
#include <stdio.h>
#include <assert.h>

#include "RenderEvent.h"

extern "C" {
#include "kgl.h"
}

#define PAGE_FLIPPING           1
#define TEXTURE_TWIDDLING       0
#define TEXTURE_EVICTION        0
#define PREFER_CLAMPING         0
#define ONLY_GENERIC_BLITTERS   0
#define CONSERVE_MEMORY         1
#define LOG_ERRORS              1
#define LOG_DEVICE_CAPS         0
#define LOG_DISPLAY_MODES       0
#define LOG_SHUTDOWN            0
#define LOG_TEXTURE_FORMATS     0
#define IMPLICIT_BEGINS         1
#define INSCENE_CHECKING        0
#define WILL_ANTIALIAS          0
#define WILL_WBUFFER            0
#define ONLY_SHARED_PALETTES    0
#define EARLY_SHARED_ATTACHMENT 1
#define DEEP_DB                 0

#define CONSIDER_SQUARE_ASPECT  0 /* 0 means ignore wild square-only aspect discrepencies */
#define MAX_SQUARE_ASPECT       4 /* will this produce images too lo-res ? */

#define HAL_GUID IID_IDirect3DHALDevice
#define RGB_GUID IID_IDirect3DRGBDevice
#define REF_GUID IID_IDirect3DRefDevice

#define PRIMARY_GUID HAL_GUID

#if LOG_ERRORS
#define errLog errLogFn
#else
#define errLog errLogNull
#endif

#if LOG_SHUTDOWN
#define LOG logFn
#else
#define LOG logNull
#endif

#define RGBA_MAKE_ARRAY(C) RGBA_MAKE(C[0], C[1], C[2], C[3])
#define PAL_MAKE(r,g,b,a) (((a) << 24) | ((b) << 16) | ((g) << 8) | (r))

typedef struct d3d_context
{
    ComPtr<IDirect3DSurface9> BackSurface;
    ComPtr<IDirect3DSurface9> DepthSurface;
    ComPtr<IDirect3D9> d3dObject;
    ComPtr<IDirect3DDevice9> d3dDevice;
    GUID                     d3dDeviceGUID;
    RECT                     ScreenRect, ViewportRect;
    HWND                     hwnd;

    D3DFORMAT texCOLORINDEX;

    GLboolean   canRenderWindowed;
    GLboolean   Fullscreen;
    GLboolean   inScene;
    GLboolean   canGammaControl;
    D3DGAMMARAMP awOldLUT;

    GLenum      Primitive;
    GLint       vertexMode;

    GLboolean   TexEnabled;
    GLboolean   DepthTest;
    GLenum      DepthFunc;
    GLboolean   DepthWrite;
    GLenum      ShadeModel;
    GLboolean   Blend;
    GLenum      BlendSrc, BlendDst;
    GLboolean   AlphaTest;
    GLenum      AlphaFunc;
    GLubyte     AlphaByteRef;
    GLboolean   LineSmooth;
    GLenum      CullFace;

    GLenum      texWrapS;
    GLenum      texWrapT;
    GLenum      texMinFilter;
    GLenum      texMagFilter;

    D3DTEXTUREOP colorOp;
    D3DTEXTUREOP alphaOp;

    D3DCOLOR    Monocolor;

    GLint       maxTexAspect;
    GLint       minTexWidth, minTexHeight;
    GLint       maxTexWidth, maxTexHeight;

    GLuint      canTexModulate;
    GLuint      canTexSelectArg1;
    GLuint      canTexAdd;

    GLuint      canAntialiasTriEdges;
    GLuint      canAntialiasTriDep;
    GLuint      canAntialiasTriIndep;
    GLuint      canAntialiasLineEdges;
    GLuint      canAntialiasLineDep;
    GLuint      canAntialiasLineIndep;
    GLuint      canZCmpLess;
    GLuint      canZCmpLessEqual;
    GLboolean   canAlphaBlend;
    GLuint      canSrcBlendSrcAlpha;
    GLuint      canSrcBlendOne;
    GLuint      canSrcBlendZero;
    D3DBLEND    srcBlendFallback;
    GLuint      canDestBlendInvSrcAlpha;
    GLuint      canDestBlendOne;
    GLuint      canDestBlendZero;
    D3DBLEND    destBlendFallback;
    GLboolean   canAlphaTest;
    GLuint      canAlphaTestGreater;
    GLuint      canAlphaTestLess;
    D3DCMPFUNC  alphaTestFallback;
    GLuint      squareOnly;
    GLuint      canFilterLinear;
    GLuint      canFilterNearest;
    GLuint      canClamp;
    GLuint      canWrap;
} d3d_context;

typedef struct d3d_texobj
{
    GLsizei              width = 0;
    GLsizei              height = 0;
    GLboolean            valid = false;
    GLboolean            paletted = false;
    IDirect3DSurface9* texSurface = nullptr;
    IDirect3DTexture9* texObj = nullptr;

    GLenum      texWrapS = 0;
    GLenum      texWrapT = 0;
    GLenum      texMinFilter = 0;
    GLenum      texMagFilter = 0;
} d3d_texobj;

extern GLcontext* CTX;
extern d3d_context* D3D;

GLboolean d3d_begin_scene(d3d_context* d3d);
GLboolean d3d_end_scene(d3d_context* d3d);

void d3d_shutdown(GLcontext* ctx);

void texbind(gl_texture_object*);
void teximg(gl_texture_object*, GLint, GLint);
void texpalette(gl_texture_object*);
void texdel(gl_texture_object*);

void errLogFn(const char* s, HRESULT hr);
void errLogNull(const char* s, HRESULT hr);

void logFn(const char* s);
void logNull(const char* s);

WORD GetNumberOfBits(DWORD mask);
GLuint GetShiftBits(GLuint mask);

#endif
