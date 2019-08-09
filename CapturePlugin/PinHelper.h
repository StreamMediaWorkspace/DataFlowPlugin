#pragma once
#include <atlcomcli.h>
#include <dshow.h>
namespace PinHelper
{
    CComPtr<IPin> GetPin(IBaseFilter *pFilter, LPCOLESTR pinname);
};

