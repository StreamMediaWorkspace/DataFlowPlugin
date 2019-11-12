#include "DShowCapture.h"
#include "../common/Utils.h"
#include "../common/Logger.h"

DShowCapture::DShowCapture(MediaDataCallbackBase *pMediaDataCallback) {
    m_pMediaDataCallback = pMediaDataCallback;
}

int DShowCapture::SetDevice(const std::string &deviceID) {
    m_deviceID = deviceID;
    return 0;
}

int DShowCapture::Start() {
    if (!m_pGraph) {
        return -1;
    }

    HRESULT hr = m_pGraph->QueryInterface(&m_mediaControl);
    if (!SUCCEEDED(hr)) {
        LogE("error: start QueryInterface %ld\n", hr);
        return -1;
    }

    if (!m_mediaControl) {
        return -1;
    }
    hr = m_mediaControl->Run();
    if (SUCCEEDED(hr)) {
        return 0;
    }

    LogE("error: start run %ld\n", hr);
    return -1;
}

int DShowCapture::Stop() {
    HRESULT hr = m_mediaControl->Stop();
    if (m_pDeviceFilter) {
        m_pDeviceFilter.Release();
    }

    if (m_mediaControl) {
        m_mediaControl.Release();
    }

    if (m_pBuilder2) {
        m_pBuilder2.Release();
    }

    if (m_pGraph) {
        m_pGraph.Release();
    }

    if (SUCCEEDED(hr)) {
        return 0;
    }
    return -1;
}

bool DShowCapture::SetCapDevice(const IID& deviceIID, const std::string& comObjID) {
    if (m_deviceID.empty()) {
        LogE("m_deviceID\n");
        return false;
    }

    CComPtr<ICreateDevEnum> create_dev_enum;
    create_dev_enum.CoCreateInstance(CLSID_SystemDeviceEnum);

    CComPtr<IEnumMoniker> enum_moniker;
    create_dev_enum->CreateClassEnumerator(deviceIID, &enum_moniker, 0);

    enum_moniker->Reset();

    IBindCtx* bind_ctx;
    HRESULT hr = ::CreateBindCtx(0, &bind_ctx);
    if (hr != S_OK) {
        return false;
    }

    ULONG ulFetched;
    CComPtr<IMoniker> moniker;
    hr = ::MkParseDisplayName(bind_ctx, Utils::String2WString(comObjID).c_str(), &ulFetched, &moniker);
    SAFE_RELEASE(bind_ctx);

    if (hr != S_OK) {
        return false;
    }

    hr = moniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&m_pDeviceFilter);
    if (hr != S_OK) {
        return false;
    }

    moniker.Release();
    create_dev_enum.Release();
    enum_moniker.Release();

    return true;
}

DShowCapture::~DShowCapture() {
    if (m_pDeviceFilter) {
        m_pDeviceFilter.Release();
    }
}

STDMETHODIMP DShowCapture::SampleCB(double SampleTime, IMediaSample *pSample) {
    return S_OK;
}

STDMETHODIMP DShowCapture::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) {
    static DWORD startTime = GetTickCount();
    OnData(GetTickCount() - startTime, pBuffer, BufferLen);
//     SampleTime *= 1000;
//     LogI("SampleTime=%f\n", SampleTime);
//     long sz = BufferLen;
//     unsigned char *buf = new unsigned char[BufferLen];
//     memcpy(buf, pBuffer, BufferLen);
//     if (m_pMediaDataCallback) {
//         m_pMediaDataCallback->OnVideoData(new VideoDataBuffer(buf, sz, 640, 480, SampleTime));
//     }
    return S_OK;
}
