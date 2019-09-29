#include "PinHelper.h"
#include <string>
#include <vector>
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

CComPtr<IPin> PinHelper::FindOutputPin(CComPtr<IBaseFilter> &filter, const GUID &type)
{
    HRESULT  hr;

    if (filter == NULL)
        return NULL;

    // find		source
    CComPtr<IEnumPins>  pEnumPins;
    hr = filter->EnumPins(&pEnumPins.p);
    if (FAILED(hr))
        return NULL;

    pEnumPins->Reset();

    CComPtr<IPin>  pOutPin;
    ULONG          pcFetched;
    while ((hr = pEnumPins->Next(1, &pOutPin.p, &pcFetched)) == S_OK)
    {
        PIN_INFO  pinInfo;
        hr = pOutPin->QueryPinInfo(&pinInfo);

        if (SUCCEEDED(hr))
        {
            //std::wstring  strInfo(pinInfo.achName);
            //sfwDbgPrint ("\tpinInfo: %s\n", (const char*)strInfo);  

            pinInfo.pFilter->Release();
            if (PinMatches(pOutPin, type, PIN_CATEGORY_CAPTURE, PINDIR_OUTPUT))
            {
                break;
            }
        }
        pOutPin.Release();
    }

    return pOutPin;
}

bool PinHelper::PinMatches(IPin *pin, const GUID &type, const GUID &category, PIN_DIRECTION dir)
{
    if (!PinHasMajorType(pin, type))
        return false;
    if (!PinIsDirection(pin, dir))
        return false;
    if (!PinIsCategory(pin, category))
        return false;

    return true;
}

bool PinHelper::PinHasMajorType(IPin *pin, const GUID &type) {
    HRESULT hr;
    AM_MEDIA_TYPE *ptr;
    CComPtr<IEnumMediaTypes> mediaEnum;

    /* first, check the config caps. */
    if (PinConfigHasMajorType(pin, type))
        return true;

    /* then let's check the media type for the pin */
    if (FAILED(pin->EnumMediaTypes(&mediaEnum)))
        return false;

    ULONG curVal;
    hr = mediaEnum->Next(1, &ptr, &curVal);
    if (hr != S_OK)
        return false;

    return ptr->majortype == type;
}

bool PinHelper::PinConfigHasMajorType(IPin *pin, const GUID &type)
{
    CComPtr<IAMStreamConfig> config;
    int count = 0;
    int size = 0;

    HRESULT hr = pin->QueryInterface(IID_IAMStreamConfig, (void**)&config);
    if (FAILED(hr))
        return false;

    hr = config->GetNumberOfCapabilities(&count, &size);
    if (FAILED(hr))
        return false;

    std::vector<BYTE> caps;
    caps.resize(size);

    for (int i = 0; i < count; i++) {
        AM_MEDIA_TYPE *ptr;
        if (SUCCEEDED(config->GetStreamCaps(i, &ptr, caps.data()))) {
            if (ptr->majortype == type)
                return true;
        }
    }

    return false;
}

bool PinHelper::PinIsDirection(IPin *pin, PIN_DIRECTION dir)
{
    if (!pin)
        return false;

    PIN_DIRECTION pinDir;
    return (SUCCEEDED(pin->QueryDirection(&pinDir)) && pinDir == dir);
}

bool PinHelper::PinIsCategory(IPin *pin, const GUID &category)
{
    if (!pin)
        return false;

    GUID pinCategory;
    HRESULT hr = GetPinCategory(pin, pinCategory);

    /* if the pin has no category interface, chances are we created it */
    if (FAILED(hr))
        return (hr == E_NOINTERFACE);

    return (category == pinCategory);
}

HRESULT PinHelper::GetPinCategory(IPin *pin, GUID &category)
{
    if (!pin)
        return E_POINTER;

    CComQIPtr<IKsPropertySet>  propertySet(pin);
    DWORD                     size;

    if (propertySet == NULL)
        return E_NOINTERFACE;

    return (propertySet->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
        NULL, 0, &category, sizeof(GUID), &size));
}