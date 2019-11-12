#pragma once
#include <atlcomcli.h>
#include <control.h>
#include <strmif.h>
#include <list>
#include <qedit.h>
#include "DShowHelper.h"
#include "../common/PluginBase.h"
class MediaDataCallbackBase {
public:
    virtual void OnData(DataBuffer *pDataBuffer) = 0;
};

#define CHECK_HR(s, text) if (FAILED(s)) {LogE("%s\n", text);return -1;} 
#define CHECK_HR_EX(s, text) if (FAILED(s)) {LogE("%s\n", text);return NULL;}

class DShowCapture : public ISampleGrabberCB {
public:
    DShowCapture(MediaDataCallbackBase *pMediaDataCallback);
    ~DShowCapture();

    int SetDevice(const std::string &deviceID);

    virtual int Init() = 0;
    virtual int Start();
    virtual int Stop();

protected:
    bool SetCapDevice(const IID& deviceIID, const std::string& comObjID);

    virtual void OnData(double SampleTime, BYTE *pBuffer, long BufferLen) = 0;

private:
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
        if (NULL == ppv) return E_POINTER;
        if (riid == __uuidof(IUnknown)) {
            *ppv = static_cast<IUnknown*>(this);
            return S_OK;
        }
        if (riid == __uuidof(ISampleGrabberCB)) {
            *ppv = static_cast<ISampleGrabberCB*>(this);
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return S_OK; }
    STDMETHODIMP_(ULONG) Release() { return S_OK; }

    STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);
    STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);

protected:
    std::string m_deviceID = "";
    CComPtr<IBaseFilter> m_pDeviceFilter = nullptr;

    CComPtr<ICaptureGraphBuilder2> m_pBuilder2 = nullptr;
    CComPtr<IGraphBuilder> m_pGraph = nullptr;
    CComQIPtr<IMediaControl> m_mediaControl = nullptr;

    MediaDataCallbackBase *m_pMediaDataCallback = nullptr;
};

