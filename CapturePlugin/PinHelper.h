#pragma once
#include <atlcomcli.h>
#include <dshow.h>
class PinHelper
{
public:
    static CComPtr<IPin> GetPin(IBaseFilter *pFilter, LPCOLESTR pinname);
    static bool PinMatches(IPin *pin, const GUID &type, const GUID &category,
        PIN_DIRECTION dir);
    static CComPtr<IPin> FindOutputPin(CComPtr<IBaseFilter> &filter, const GUID &type);
private:
    static bool PinHasMajorType(IPin *pin, const GUID &type);
    static bool PinIsDirection(IPin *pin, PIN_DIRECTION dir);
    static bool PinIsCategory(IPin *pin, const GUID &category);
    static bool PinConfigHasMajorType(IPin *pin, const GUID &type);
    static HRESULT GetPinCategory(IPin *pin, GUID &category);
};

