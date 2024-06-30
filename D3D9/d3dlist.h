/*=============================================================================
    Name    : d3dlist.h
    Purpose : lists used by rGL's Direct3D driver

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _D3DLIST_H
#define _D3DLIST_H

using namespace std;

typedef struct devlist_s
{
    D3DCAPS9 caps;
    GUID   guid;
    LPGUID pGuid;
    char ddName[128];
    char descStr[128];
    char nameStr[128];
} devlist_t;

typedef list<devlist_t> listdev;
typedef vector<D3DFORMAT> listpf;

extern listdev devList;
extern listpf texList;
extern listpf loDepthList;
extern listpf hiDepthList;

#endif
