#pragma once
#include <atlcomcli.h>
#include <control.h>
#include <strmif.h>

#include <qedit.h>

class MediaDataCallbackBase {
public:
    virtual void OnVideoData(void *data, int len) = 0;
    virtual void OnAudioData(void *data, int len) = 0;
};

class DShowCapture : public ISampleGrabberCB
{
public:
    enum CaptureType { euVideo, euAudio };

    DShowCapture(CaptureType type, MediaDataCallbackBase *pMediaDataCallback);
    ~DShowCapture();

    int Run();
    int Stop();
    /*摄像头相关start*/
private:
    HRESULT BuildGraph();
    CComPtr<IBaseFilter> GetFirstDevice(const IID &clsidDeviceClass);

    /*数据回调start*/
private:
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
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

    //ISampleGrabberCB
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);
    STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);
    /*数据回调end*/

private:
    CComPtr<ICaptureGraphBuilder2> m_pBuilder2 = nullptr;
    CComPtr<IGraphBuilder> m_pGraph = nullptr;
    CComQIPtr<IMediaControl> m_mediaControl = nullptr;
    CComPtr<IBaseFilter> m_pSrcFilter = nullptr;
    CaptureType m_captureType;

    MediaDataCallbackBase *m_pMediaDataCallback = nullptr;
};

