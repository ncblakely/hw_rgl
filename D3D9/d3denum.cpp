/*=============================================================================
    Name    : d3denum.cpp
    Purpose : Direct3D enumeration functions

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"
#include "d3dlist.h"
#include "d3dinit.h"

void enum_d3d_devices(d3d_context* d3d)
{
    auto pD3D = d3d->d3dObject;

    UINT adapterCount = d3d->d3dObject->GetAdapterCount();

    for (UINT i = 0; i < adapterCount; ++i)
    {
        devlist_t dt;

        D3DADAPTER_IDENTIFIER9 adapterIdentifier;
        HRESULT hr = pD3D->GetAdapterIdentifier(i, 0, &adapterIdentifier);
        if (FAILED(hr))
            continue;

        hr = pD3D->GetDeviceCaps(i, D3DDEVTYPE_HAL, &dt.caps);
        if (FAILED(hr))
            continue;

        strcpy_s(dt.ddName, adapterIdentifier.DeviceName);
        strcpy_s(dt.descStr, adapterIdentifier.Description);
        strcpy_s(dt.nameStr, adapterIdentifier.DeviceName);

        devList.push_back(dt);
    }
}

void enum_dd_devices(d3d_context* d3d)
{
    char targetDev[128];

    devList.erase(devList.begin(), devList.end());

    enum_d3d_devices(d3d);

    // TODO D3D9: This doesn't make any sense anymore -- remove this?
    for (auto i = devList.begin(); i != devList.end(); ++i)
    {
        strncpy(targetDev, rglD3DGetDevice(), 127);
        if (strlen(targetDev) > 0)
        {
            if (strcmp((*i).nameStr, targetDev) == 0)
            {
//                rglD3DSetDevice(NULL);
                break;
            }
        }
    }
}