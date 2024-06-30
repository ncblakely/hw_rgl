/*=============================================================================
    Name    : d3dtex.cpp
    Purpose : Direct3D texture-related functions

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"
#include "d3dlist.h"
#include "d3dblt.h"
#include "d3dtex.h"

#define II (*i)

#define DDPF_ALLPALETTED (DDPF_PALETTEINDEXED8 | DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXEDTO8)

static std::vector<D3DFORMAT> s_allD3DFormats =
{
    D3DFMT_R8G8B8,
    D3DFMT_A8R8G8B8,
    D3DFMT_X8R8G8B8,
    D3DFMT_R5G6B5,
    D3DFMT_X1R5G5B5,
    D3DFMT_A1R5G5B5,
    D3DFMT_A4R4G4B4,
    D3DFMT_R3G3B2,
    D3DFMT_A8,
    D3DFMT_A8R3G3B2,
    D3DFMT_X4R4G4B4,
    D3DFMT_A2B10G10R10,
    D3DFMT_A8B8G8R8,
    D3DFMT_X8B8G8R8,
    D3DFMT_G16R16,
    D3DFMT_A2R10G10B10,
    D3DFMT_A16B16G16R16,
    D3DFMT_A8P8,
    D3DFMT_P8,
    D3DFMT_L8,
    D3DFMT_A8L8,
    D3DFMT_A4L4,
    D3DFMT_V8U8,
    D3DFMT_L6V5U5,
    D3DFMT_X8L8V8U8,
    D3DFMT_Q8W8V8U8,
    D3DFMT_V16U16,
    D3DFMT_A2W10V10U10,
    D3DFMT_UYVY,
    D3DFMT_YUY2,
    D3DFMT_DXT1,
    D3DFMT_DXT2,
    D3DFMT_DXT3,
    D3DFMT_DXT4,
    D3DFMT_DXT5,
    D3DFMT_D16_LOCKABLE,
    D3DFMT_D32,
    D3DFMT_D15S1,
    D3DFMT_D24S8,
    D3DFMT_D24X8,
    D3DFMT_D24X4S4,
    D3DFMT_D16,
    D3DFMT_D32F_LOCKABLE,
    D3DFMT_D24FS8,
    D3DFMT_D32_LOCKABLE,
    D3DFMT_S8_LOCKABLE,
    D3DFMT_L16,
    D3DFMT_VERTEXDATA,
    D3DFMT_INDEX16,
    D3DFMT_INDEX32,
    D3DFMT_Q16W16V16U16,
    D3DFMT_MULTI2_ARGB8,
    D3DFMT_R16F,
    D3DFMT_G16R16F,
    D3DFMT_A16B16G16R16F,
    D3DFMT_R32F,
    D3DFMT_G32R32F,
    D3DFMT_A32B32G32R32F,
    D3DFMT_CxV8U8,
    D3DFMT_A1,
    D3DFMT_A2B10G10R10_XR_BIAS,
    D3DFMT_BINARYBUFFER
};

void enum_texture_formats(d3d_context* d3d, D3DFORMAT adapterFormat)
{
    texList.clear();

    for (auto format : s_allD3DFormats)
    {
        HRESULT hr = d3d->d3dObject->CheckDeviceFormat(0, D3DDEVTYPE_HAL, adapterFormat, 0, D3DRTYPE_TEXTURE, format);

        if (SUCCEEDED(hr))
        {
            texList.push_back(format);
        }
    }
}

void d3d_rescale_scanline(GLsizei bytes, GLubyte* dData, GLsizei dWidth, GLubyte* sData, GLsizei sWidth)
{
    GLint dup, skip;
    GLint sx, dx;
    GLint i, j;

    if (dWidth > sWidth)
    {
        //dup x
        dup = dWidth / sWidth;
        for (sx = dx = 0; sx < bytes*sWidth; sx += bytes)
        {
            for (j = 0; j < dup; j++, dx += bytes)
            {
                for (i = 0; i < bytes; i++)
                {
                    dData[dx+i] = sData[sx+i];
                }
            }
        }
    }
    else if (sWidth > dWidth)
    {
        //skip x
        skip = sWidth / dWidth;
        for (sx = dx = 0; sx < bytes*sWidth; sx += bytes*skip, dx += bytes)
        {
            for (i = 0; i < bytes; i++)
            {
                dData[dx+i] = sData[sx+i];
            }
        }
    }
    else
    {
        //same x
        MEMCPY(dData, sData, bytes*dWidth);
    }
}

void d3d_rescale_generic(
    GLsizei bytes,
    GLubyte* dData, GLsizei dWidth, GLsizei dHeight,
    GLubyte* sData, GLsizei sWidth, GLsizei sHeight)
{
    GLubyte* pd;
    GLubyte* ps;
    GLint dy, sy;
    GLint dup, skip;
    GLint i;

    if (dHeight > sHeight)
    {
        //dup y
        dup = dHeight / sHeight;
        for (sy = dy = 0; sy < sHeight; sy++)
        {
            ps = sData + bytes*sy*sWidth;
            for (i = 0; i < dup; i++, dy++)
            {
                pd = dData + bytes*dy*dWidth;
                d3d_rescale_scanline(bytes, pd, dWidth, ps, sWidth);
            }
        }
    }
    else if (sHeight > dHeight)
    {
        //skip y
        skip = sHeight / dHeight;
        for (sy = dy = 0; sy < sHeight; sy += skip, dy++)
        {
            pd = dData + bytes*dy*dWidth;
            ps = sData + bytes*sy*sWidth;
            d3d_rescale_scanline(bytes, pd, dWidth, ps, sWidth);
        }
    }
    else
    {
        //same y
        for (sy = 0; sy < sHeight; sy++)
        {
            pd = dData + bytes*sy*dWidth;
            ps = sData + bytes*sy*sWidth;
            d3d_rescale_scanline(bytes, pd, dWidth, ps, sWidth);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_blt_texture
    Description :
    Inputs      : tex - GL texture object
                  ddpf - pixelformat of texture surface (not needed with ONLY_GENERIC_BLITTERS)
    Outputs     : currently bound texture object's surface should contain a valid image
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
GLboolean d3d_blt_texture(gl_texture_object* tex, D3DFORMAT ddpf)
{
    d3d_texobj* t3d = (d3d_texobj*)tex->DriverData;
    GLubyte* data;
    GLubyte* tempData;
    GLsizei width, height, bytes;
    GLboolean result;

    // ASSERT_UNTESTED();

    tempData = NULL;

    if (t3d->width != 0 && t3d->height != 0)
    {
        //aspect-corrected texture
        width  = t3d->width;
        height = t3d->height;
        switch (tex->Format)
        {
        case GL_COLOR_INDEX:
            bytes = 1;
            break;
        case GL_RGBA16:
            bytes = 2;
            break;
        case GL_RGB:
        case GL_RGBA:
            bytes = 4;
            break;
        default:
            return FALSE;
        }
        tempData = new GLubyte[bytes * width * height];
        d3d_rescale_generic(bytes, tempData, width, height, tex->Data, tex->Width, tex->Height);
        data = tempData;
    }
    else
    {
        //normal texture
        width  = tex->Width;
        height = tex->Height;
        data = tex->Data;
    }

    result = FALSE;

    switch (tex->Format)
    {
    case GL_COLOR_INDEX:
        result = d3d_blt_COLORINDEX(t3d->texSurface, data, width, height);
        break;

    case GL_RGB:
    case GL_RGBA:
#if ONLY_GENERIC_BLITTERS
        result = d3d_blt_RGBA_generic(t3d->texSurface, data, width, height);
#else
        switch (ddpf)
        {
        case D3DFMT_R5G6B5:
            ASSERT_UNTESTED();
            result = d3d_blt_RGBA_0565(t3d->texSurface, data, width, height);
            break;
        case D3DFMT_X1R5G5B5: // TODO D3D9: This is probably the wrong format!
            ASSERT_UNTESTED();
            result = d3d_blt_RGBA_0555(t3d->texSurface, data, width, height);
            break;
        case D3DFMT_A4R4G4B4:
            ASSERT_UNTESTED();
            result = d3d_blt_RGBA_4444(t3d->texSurface, data, width, height);
            break;
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
            result = d3d_blt_RGBA_8888(t3d->texSurface, data, width, height);
            break;
        default:
            assert(false && "Unhandled format");
            result = d3d_blt_RGBA_generic(t3d->texSurface, data, width, height);
        }
#endif
        break;

    case GL_RGBA16:
#if ONLY_GENERIC_BLITTERS
        result = d3d_blt_RGBA16_generic(t3d->texSurface, data, width, height);
#else
        switch (ddpf)
        {
        case D3DFMT_A4R4G4B4:
            result = d3d_blt_RGBA16_4444(t3d->texSurface, data, width, height);
            break;
        case D3DFMT_A8R8G8B8:
            result = d3d_blt_RGBA16_8888(t3d->texSurface, data, width, height);
            break;
        default:
            assert(false && "Unhandled format for blit");
            result = d3d_blt_RGBA16_generic(t3d->texSurface, data, width, height);
        }
#endif
    }

    if (tempData != NULL)
    {
        delete [] tempData;
    }

    return result;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_create_texture
    Description : creates a D3D rep of a GL texture
    Inputs      : tex - GL texture object
    Outputs     : the driver-specific portion of the texobj will be filled in
    Return      : TRUE (success) or FALSE (failure)
----------------------------------------------------------------------------*/
GLboolean d3d_create_texture(gl_texture_object* tex)
{
    HRESULT hr;
    d3d_texobj* t3d;
    GLsizei width, height;
    GLint maxAspect;
    D3DFORMAT ddpf;

    t3d = (d3d_texobj*)tex->DriverData;
    t3d->paletted = GL_FALSE;
    t3d->width = 0;     //default to no driver-specific scaling
    t3d->height = 0;

    switch (tex->Format)
    {
        case GL_COLOR_INDEX:
            ASSERT_UNTESTED();

            t3d->paletted = GL_TRUE;
            ddpf = D3DFMT_P8;
            break;
        case GL_RGBA16:
            ddpf = D3DFMT_A4R4G4B4;
            break;
        case GL_RGBA:
            ddpf = D3DFMT_A8R8G8B8;
            break;
        case GL_RGB:
            ddpf = D3DFMT_X8R8G8B8;
            break;
        default:
            return GL_FALSE;
    }

    // TODO DX9: Remove aspect ratio adjustment
    //handle aspect-ratio adjustments
    if (D3D->squareOnly)
    {
        maxAspect = 1;
    }
    else
    {
        maxAspect = D3D->maxTexAspect;
    }
    if (maxAspect == 0)
    {
        //no aspect restriction
        width  = tex->Width;
        height = tex->Height;

        //driver min-max cap
        if (D3D->maxTexWidth != 0)
            if (width > D3D->maxTexWidth) width = D3D->maxTexWidth;
        if (D3D->minTexWidth != 0)
            if (width < D3D->minTexWidth) width = D3D->minTexWidth;
        if (D3D->maxTexHeight != 0)
            if (height > D3D->maxTexHeight) height = D3D->maxTexHeight;
        if (D3D->minTexHeight != 0)
            if (height < D3D->minTexHeight) height = D3D->minTexHeight;

        if (width != tex->Width || height != tex->Height)
        {
            t3d->width  = width;
            t3d->height = height;
        }
    }
    else
    {
        if (maxAspect == 1)
        {
            //square-only restriction
            width  = tex->Width;
            height = tex->Height;

            //enforce a max aspect to minimize wasted space
            if (height > width)
            {
#if CONSIDER_SQUARE_ASPECT
                while ((height / width) > MAX_SQUARE_ASPECT)
                {
                    height >>= 1;
                }
#endif
                width = height;
            }
            else if (width > height)
            {
#if CONSIDER_SQUARE_ASPECT
                while ((width / height) > MAX_SQUARE_ASPECT)
                {
                    width >>= 1;
                }
#endif
                height = width;
            }

            //driver min/max cap
            if (D3D->maxTexWidth != 0)
            {
                if (width > D3D->maxTexWidth)
                {
                    width  = D3D->maxTexWidth;
                    height = width;
                }
            }
            if (D3D->minTexWidth != 0)
            {
                if (width < D3D->minTexWidth)
                {
                    width  = D3D->minTexWidth;
                    height = width;
                }
            }

            if (width != tex->Width || height != tex->Height)
            {
                t3d->width  = width;
                t3d->height = height;
            }
        }
        else
        {
            //arbitrary aspect restriction
            width  = tex->Width;
            height = tex->Height;

            if (width > height)
            {
                while ((width / height) > maxAspect)
                {
                    width  >>= 1;
                }
            }
            else if (height > width)
            {
                while ((height / width) > maxAspect)
                {
                    height >>= 1;
                }
            }

            //driver min/max cap
            if (D3D->maxTexWidth != 0)
                if (width > D3D->maxTexWidth) width = D3D->maxTexWidth;
            if (D3D->minTexWidth != 0)
                if (width < D3D->minTexWidth) width = D3D->minTexWidth;
            if (D3D->maxTexHeight != 0)
                if (height > D3D->maxTexHeight) height = D3D->maxTexHeight;
            if (D3D->minTexHeight != 0)
                if (height < D3D->minTexHeight) height = D3D->minTexHeight;

            if (width != tex->Width || height != tex->Height)
            {
                t3d->width  = width;
                t3d->height = height;
            }
        }
    }

    hr = D3D->d3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, ddpf, D3DPOOL_DEFAULT, &t3d->texObj, nullptr);
    if (FAILED(hr))
    {
        char estring[1024], texstring[16];
        switch (tex->Format)
        {
        case GL_COLOR_INDEX:
            sprintf(texstring, "COLOR_INDEX");
            break;
        case GL_RGBA16:
            sprintf(texstring, "RGBA16");
            break;
        case GL_RGBA:
            sprintf(texstring, "RGBA");
            break;
        case GL_RGB:
            sprintf(texstring, "RGB");
            break;
        default:
            sprintf(texstring, "???");
        }
        sprintf(estring,
                "d3d_create_texture(%s:CreateSurface[%d.%d was %d.%d] ... w %d.%d, h %d.%d, a %d)",
                texstring,
                t3d->width, t3d->height,
                tex->Width, tex->Height,
                D3D->minTexWidth, D3D->maxTexWidth,
                D3D->minTexHeight, D3D->maxTexHeight,
                D3D->squareOnly ? 1 : D3D->maxTexAspect);
        errLog(estring, hr);
        return GL_FALSE;
    }

#if EARLY_SHARED_ATTACHMENT
    if (CTX->UsingSharedPalette && tex->Format == GL_COLOR_INDEX)
    {
        d3d_attach_shared_palette(tex);
    }
#endif

    hr = t3d->texObj->GetSurfaceLevel(0, &t3d->texSurface);
    if (FAILED(hr))
    {
        t3d->texSurface->Release();
        t3d->texSurface = NULL;
        errLog("d3d_create_texture(GetSurfaceLevel)", hr);
        return GL_FALSE;
    }

    return d3d_blt_texture(tex, ddpf);
}

/*-----------------------------------------------------------------------------
    Name        : d3d_free_texture
    Description : releases the interface and surface
    Inputs      : t3d - D3D texobj
    Outputs     : t3d->valid == GL_FALSE
    Return      :
----------------------------------------------------------------------------*/
void d3d_free_texture(d3d_texobj* t3d)
{
    if (t3d->texObj != NULL)
    {
        t3d->texObj->Release();
        t3d->texObj = NULL;
    }

    if (t3d->texSurface != NULL)
    {
        t3d->texSurface->Release();
        t3d->texSurface = NULL;
    }

    t3d->valid = GL_FALSE;
}

//allocate a D3D texobj
d3d_texobj* d3d_alloc_texobj(void)
{
    d3d_texobj* t3d;

    t3d = new d3d_texobj;
    t3d->valid = GL_FALSE;
    t3d->texSurface = NULL;
    t3d->texObj = NULL;
    t3d->width = t3d->height = 0;

    return t3d;
}

//set defaults in a D3D texobj
void d3d_fill_texobj(gl_texture_object* tex)
{
    d3d_texobj* t3d = (d3d_texobj*)tex->DriverData;

    t3d->texWrapS = GL_REPEAT;
    t3d->texWrapT = GL_REPEAT;
    t3d->texMinFilter = GL_NEAREST;
    t3d->texMagFilter = GL_NEAREST;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_bind_texture
    Description : sets the texture used by the rendering device
    Inputs      : t3d - D3D texobj
    Outputs     :
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_bind_texture(d3d_texobj* t3d)
{
    HRESULT hr;

    if ((!t3d->valid) || (t3d->texObj == NULL))
    {
        D3D->d3dDevice->SetTexture(0, NULL);
        return GL_FALSE;
    }

    hr = D3D->d3dDevice->SetTexture(0, t3d->texObj);
    if (FAILED(hr))
    {
        errLog("d3d_bind_texture(SetTexture)", hr);
        return GL_FALSE;
    }
    else
    {
        return GL_TRUE;
    }
}

//create D3D reps of all textures in the GL
void d3d_load_all_textures(GLcontext* ctx)
{
    GLuint i;
    hashtable* table;
    hash_t* element;
    gl_texture_object* tex;

    table = rglGetTexobjs();
    if (table == NULL || table->maxkey == 0)
    {
        return;
    }
    for (i = 0; i < TABLE_SIZE; i++)
    {
        element = table->table[i];
        while (element != NULL)
        {
            tex = (gl_texture_object*)element->data;
            if (tex != NULL && tex->created)
            {
                texbind(tex);
                teximg(tex, 0, tex->Format);
                if (tex->Format == GL_COLOR_INDEX)
                {
                    texpalette(tex);
                }
            }

            //next chained element
            element = element->next;
        }
    }
}

//remove D3D reps from all textures in the GL
void d3d_free_all_textures(GLcontext* ctx)
{
    GLuint i;
    hashtable* table;
    hash_t* element;
    gl_texture_object* tex;

    table = rglGetTexobjs();
    if (table == NULL || table->maxkey == 0)
    {
        return;
    }
    for (i = 0; i < TABLE_SIZE; i++)
    {
        element = table->table[i];
        while (element != NULL)
        {
            tex = (gl_texture_object*)element->data;
            if (tex != NULL)
            {
                texdel(tex);
            }

            //next chained element
            element = element->next;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : d3d_create_shared_palette
    Description : create the D3D rep of the GL's shared palette.  done once
    Inputs      :
    Outputs     : D3D->sharedPalette contains a palette, already initialized but
                  perhaps simply to 0 if no shared palette is currently bound in
                  the GL
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_create_shared_palette(void)
{
    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_modify_shared_palette
    Description : modifies the colour entries in the D3D shared palette
    Inputs      :
    Outputs     : D3D->sharedPalette's entries are modified
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_modify_shared_palette(void)
{
    ASSERT_UNIMPLEMENTED();

    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_attach_shared_palette
    Description : attaches the D3D shared palette to a D3D texobj.  done once
    Inputs      : tex - GL texture object
    Outputs     : shared palette is bound to the texobj's surface
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_attach_shared_palette(gl_texture_object* tex)
{
    ASSERT_UNIMPLEMENTED();
    return GL_TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : d3d_attach_palette
    Description : attaches a palette to a D3D texobj
    Inputs      : tex - GL texture object
    Outputs     : a palette is created and bound to the texobj's surface.  any
                  previously bound palette is first dereferenced
    Return      : TRUE or FALSE
----------------------------------------------------------------------------*/
GLboolean d3d_attach_palette(gl_texture_object* tex)
{
    ASSERT_UNIMPLEMENTED();
    return GL_TRUE;
}
