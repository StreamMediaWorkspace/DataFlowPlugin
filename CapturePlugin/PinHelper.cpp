#include "PinHelper.h"
#include "../common/logger.h"

BOOL hrcheck(HRESULT hr, TCHAR* errtext) {
    if (hr >= S_OK)
        return FALSE;

    TCHAR szErr[MAX_ERROR_TEXT_LEN];
    DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
    if (res) {
        LogE("Error %x: %s\n%s\n", hr, errtext, szErr);
    }
    else {
        LogE("Error %x: %s\n", hr, errtext);
    }
    return TRUE;
}

CComPtr<IPin> PinHelper::GetPin(IBaseFilter *pFilter, LPCOLESTR pinname)
{
    CComPtr<IEnumPins> pEnum;
    CComPtr<IPin> pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);

    if (hrcheck(hr, L"Can't enumerate pins."))
        return NULL;

    while (pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_INFO pinfo;
        pPin->QueryPinInfo(&pinfo);
        BOOL found = !_wcsicmp(pinname, pinfo.achName);
        if (pinfo.pFilter) pinfo.pFilter->Release();
        if (found)
            return pPin;
        pPin.Release();
    }
    LogE("Pin not found!\n");
    return NULL;
}
