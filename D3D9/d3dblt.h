/*=============================================================================
    Name    : d3dblt.h
    Purpose : Direct3D texture blitters

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _D3DBLT_H
#define _D3DBLT_H

GLboolean d3d_blt_RGBA_generic(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA16_generic(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA_0565(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA_0555(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA_8888(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA_4444(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA16_4444(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_RGBA16_8888(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);
GLboolean d3d_blt_COLORINDEX(IDirect3DSurface9* surf, GLubyte* data, GLsizei width, GLsizei height);

void d3d_draw_quad(GLint xOfs, GLint yOfs, GLsizei width, GLsizei height, ComPtr<IDirect3DTexture9> offscreenTexture, bool reversed);

void d3d_draw_pixels_RGBA_generic(GLint xOfs, GLint yOfs, GLsizei width, GLsizei height, GLubyte* data);

void d3d_draw_pixels_RGBA_pitched(GLint x0, GLint y0, GLint x1, GLint y1,
                                  GLsizei swidth, GLsizei sheight, GLsizei spitch,
                                  GLubyte* data);

#endif
