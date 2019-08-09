#include "DShowCapture.h"
#include <qedit.h>
#include <uuids.h>
#include <amvideo.h>
#include "DShowHelper.h"
#include "PinHelper.h"
#include "../common/Logger.h"
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib,"Quartz.lib")

#define CHECK_HR(s, text) if (FAILED(s)) {LogE("%s\n", text);return -1;} 
#define CHECK_HR_EX(s, text) if (FAILED(s)) {LogE("%s\n", text);return NULL;}

#include "../common/PluginBase.h"

DShowCapture::DShowCapture(CaptureType type, MediaDataCallbackBase *pMediaDataCallback) {
    m_captureType = type;
    m_pMediaDataCallback = pMediaDataCallback;
}

int DShowCapture::Run() {
    HRESULT hr = BuildGraph();
    if (!SUCCEEDED(hr)) {
        LogE("error: start BuildGraph %ld\n", hr);
        return -1;
    }

    hr = m_pGraph->QueryInterface(&m_mediaControl);
    if (!SUCCEEDED(hr)) {
        LogE("error: start QueryInterface %ld\n", hr);
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
    if (m_pSrcFilter) {
        m_pSrcFilter.Release();
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
}

HRESULT DShowCapture::BuildGraph() {
    HRESULT hr = m_pGraph.CoCreateInstance(CLSID_FilterGraph);
    CHECK_HR(hr, "create filter graph error");
    LogI("Building graph...\n");

    if (!m_pGraph) {
        hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder, (void **)&m_pGraph);
        CHECK_HR(hr, "create graph error");
    }

    hr = m_pBuilder2.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    CHECK_HR(hr, _T("Can't create Capture Graph Builder"));
    hr = m_pBuilder2->SetFiltergraph(m_pGraph);
    CHECK_HR(hr, "Can't SetFiltergraph");

    //add USB2.0 Camera
    //CComPtr<IBaseFilter> pUSB20Camera = CreateFilterByName(L"USB2.0 Camera", CLSID_VideoCaptureSources);
    if (m_captureType == euVideo)
    {
        m_pSrcFilter = GetFirstDevice(CLSID_VideoInputDeviceCategory);
    } else {
        m_pSrcFilter = GetFirstDevice(CLSID_AudioInputDeviceCategory);
    }

    hr = m_pGraph->AddFilter(m_pSrcFilter, /*L"USB2.0 Camera"*/L"Integrated Camera");
    CHECK_HR(hr, "Can't add capture to graph");

    /*hr = m_pGraph->AddFilter(m_pMicrophoneSrc, L"mocrophone");
    CHECK_HR(hr, "Can't add mocrophone to graph");

    AM_MEDIA_TYPE pmt;
    ZeroMemory(&pmt, sizeof(AM_MEDIA_TYPE));
    pmt.majortype = MEDIATYPE_Video;
    pmt.subtype = MEDIASUBTYPE_YUY2;
    pmt.formattype = FORMAT_VideoInfo;
    pmt.bFixedSizeSamples = TRUE;
    pmt.cbFormat = 88;
    pmt.lSampleSize = 614400;
    pmt.bTemporalCompression = FALSE;
    VIDEOINFOHEADER format;
    ZeroMemory(&format, sizeof(VIDEOINFOHEADER));
    format.bmiHeader.biSize = 40;
    format.bmiHeader.biWidth = 640;
    format.bmiHeader.biHeight = 480;
    format.bmiHeader.biPlanes = 1;
    format.bmiHeader.biBitCount = 16;
    format.bmiHeader.biCompression = 844715353;
    format.bmiHeader.biSizeImage = 614400;
    pmt.pbFormat = (BYTE*)&format;
    CComQIPtr<IAMStreamConfig, &IID_IAMStreamConfig> isc(PinHelper::GetPin(m_pCameraSrc, L"捕获"));
    hr = isc->SetFormat(&pmt);
    CHECK_HR(hr, _T("Can't set format"));*/

    //add video SampleGrabber
    CComPtr<IBaseFilter> pSampleGrabber;
    hr = pSampleGrabber.CoCreateInstance(CLSID_SampleGrabber);
    CHECK_HR(hr, "Can't create video SampleGrabber");
    hr = m_pGraph->AddFilter(pSampleGrabber, L"SampleGrabber");
    CHECK_HR(hr, "Can't add SampleGrabber to graph");
    CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> pSampleGrabberCB(pSampleGrabber);

    //设置视频媒体类型
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    hr = pSampleGrabberCB->SetMediaType(&mt);

    //here we provide our callback:
    hr = pSampleGrabberCB->SetCallback(this, 0);
    CHECK_HR(hr, "Can't set callback");

    //connect USB2.0 Camera and SampleGrabber
    hr = m_pBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pSrcFilter, NULL, pSampleGrabber);
    CHECK_HR(hr, _T("Can't render stream to SampleGrabber"));

    //add audio  SampleGrabber
    //声音
    /*    DShowHelper::SetAudioFormat(m_pMicrophoneSrc, 32000, 16, 2);
    CComPtr<IBaseFilter> pAudioSampleGrabber;
    hr = pAudioSampleGrabber.CoCreateInstance(CLSID_SampleGrabber);
    CHECK_HR(hr, "Can't create SampleGrabber");
    hr = m_pGraph->AddFilter(pAudioSampleGrabber, L"AudioSampleGrabber");
    CHECK_HR(hr, "Can't add SampleGrabber to graph");
    CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> pAudioSampleGrabberCB(pAudioSampleGrabber);

    //here we provide our callback:
    hr = pAudioSampleGrabberCB->SetCallback(this, 1);
    CHECK_HR(hr, "Can't set callback");


    //connect microphone and SampleGrabber
    hr = m_pBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, m_pMicrophoneSrc, NULL, pAudioSampleGrabber);
    CHECK_HR(hr, _T("Can't render mic stream to SampleGrabber"));*/

    /*IBaseFilter *pMux;
    hr = m_pBuilder2->SetOutputFileName(
    &MEDIASUBTYPE_Avi, // Specifies AVI for the target file.
    L"D:\\Example.avi", // File name.
    &pMux, // Receives a pointer to the mux.
    NULL); // (Optional) Receives a pointer to the file sink*/

    //render the video in a window
    //hr = m_pBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, NULL, pVideoSampleGrabber, NULL, NULL);
    //CHECK_HR(hr, "Can't render stream from SampleGrabber");

    return S_OK;
}

CComPtr<IBaseFilter> DShowCapture::GetFirstDevice(const IID &clsidDeviceClass) {
    CComPtr<IBaseFilter> src = nullptr;
    ICreateDevEnum *pDevEnum = NULL;
    IEnumMoniker *pClsEnum = NULL;
    IMoniker *pMoniker = NULL;
    //创建设备枚举COM对象  
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pDevEnum);
    CHECK_HR_EX(hr, "");
    //创建视频采集设备枚举COM对象  
    hr = pDevEnum->CreateClassEnumerator(clsidDeviceClass, &pClsEnum, 0);
    CHECK_HR_EX(hr, "");

    int i = 0;
    while (i <= 0)
    {
        hr = pClsEnum->Next(1, &pMoniker, NULL);
        CHECK_HR_EX(hr, "");

        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
            (void**)(&pPropBag));
        if (FAILED(hr))
        {
            pMoniker->Release();
            continue; // Skip this one, maybe the next one will work.
        }
        // Find the description or friendly name.
        VARIANT varName;
        VariantInit(&varName);
        hr = pPropBag->Read(L"Description", &varName, 0);
        if (FAILED(hr))
        {
            hr = pPropBag->Read(L"FriendlyName", &varName, 0);
        }

        LogI("=======%s\n", varName.bstrVal);
        pPropBag->Release();
        pMoniker->Release();
        break;
        ++i;
    }

    //IBaseFilter *m_pSrc;
    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&src);//就是这句获得Filter  
    CHECK_HR_EX(hr, "");

    return src;
}

STDMETHODIMP DShowCapture::SampleCB(double SampleTime, IMediaSample *pSample) {
    if (!pSample) {
        return E_POINTER;
    }
    long sz = pSample->GetActualDataLength();
    BYTE *pBuf = NULL;
    pSample->GetPointer(&pBuf);
    if (sz <= 0 || pBuf == NULL) {
        return E_UNEXPECTED;
    }

    for (int i = 0; i < sz; i += 2) {
        pBuf[i] = 255 - pBuf[i];
    }
    //pSample->Release();

    if (m_pMediaDataCallback && m_captureType == euVideo) {
        m_pMediaDataCallback->OnVideoData(pBuf, sz);
    } else if (m_pMediaDataCallback && m_captureType == euAudio) {
        m_pMediaDataCallback->OnAudioData(pBuf, sz);
    }
    return S_OK;
}

STDMETHODIMP DShowCapture::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) {//声音
                                                                                  //fwrite(pBuffer, BufferLen, sizeof(BYTE), m_file);
    return S_OK;
}


DShowCapture::~DShowCapture()
{
}
