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

int DShowCapture::SetVideoFomat(int width, int height, int fps) {
    m_width = width;
    m_height = height;
    m_fps = fps;

    if (!m_pSrcFilter) {
        return -1;
    }
    return DShowHelper::SetVideoFomat(width, height, fps, m_pSrcFilter) == S_OK ? 0 : -1;
}
int DShowCapture::Init() {
    HRESULT hr = BuildGraph();
    if (!SUCCEEDED(hr)) {
        LogE("error: start BuildGraph %ld\n", hr);
        return -1;
    }
    return 0;
}

int DShowCapture::Run() {
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
    return -1;
}

bool DShowCapture::GetCameraSupportedResolution(std::list<VideoFormat>& frameRates) {
    return DShowHelper::GetCameraSupportedResolution(frameRates, m_pSrcFilter);
}

bool DShowCapture::SetCameraSupportedResolution(int index) {
    if (!m_pSrcFilter || !m_pBuilder2) {
        LogE("DShowCapture SetCameraSupportedResolution %p,%p\n", m_pSrcFilter, m_pBuilder2);
        return false;
    }
    return DShowHelper::SetCameraSupportedResolution(index, m_pSrcFilter, m_pBuilder2);
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
    std::wstring name;
    if (m_captureType == euVideo)
    {
        m_pSrcFilter = GetFirstDevice(CLSID_VideoInputDeviceCategory, name);
    } else {
        m_pSrcFilter = GetFirstDevice(CLSID_AudioInputDeviceCategory, name);
    }

    hr = m_pGraph->AddFilter(m_pSrcFilter, name.c_str()/*L"USB2.0 Camera"*//*L"Integrated Camera"*//*L"TTQ_HD_1080P Camera"*/);
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
//     AM_MEDIA_TYPE mt;
//     ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
//     mt.majortype = MEDIATYPE_Video;
//     mt.subtype = MEDIASUBTYPE_RGB24;  //抓取RGB24
//     mt.formattype = FORMAT_VideoInfo;
// 
//     VIDEOINFOHEADER format;
//     ZeroMemory(&format, sizeof(VIDEOINFOHEADER));
//     format.bmiHeader.biWidth = m_width;
//     format.bmiHeader.biHeight = m_height;
//     mt.pbFormat = (BYTE*)&format;

//设置视频分辨率、格式 
    IAMStreamConfig *pConfig = NULL;
    m_pBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
        m_pSrcFilter, IID_IAMStreamConfig, (void **)&pConfig);

    AM_MEDIA_TYPE *mt = NULL;
    VIDEO_STREAM_CONFIG_CAPS scc;
    pConfig->GetStreamCaps(0, &mt, (BYTE*)&scc); //nResolutionIndex就是选择的分辨率序号

    //这里仅以采集源中的两种做例子（YUY2和RGB），一般的摄像头都是支持YUY2的 
    if (mt->subtype == MEDIASUBTYPE_YUY2) {
    }
    else  //如果不是YUY2，则默认为RGB24，需要摄像头支持RGB24，否则只能针对支持的类型做处理
    {
        mt->majortype = MEDIATYPE_Video;
        mt->subtype = MEDIASUBTYPE_RGB24; //RGB24 
        mt->formattype = FORMAT_VideoInfo;

        pConfig->SetFormat(mt);
    }

    //是否缓存数据，缓存的话，可以给后面做其他处理，不缓存的话，图像处理就放在回调中
    pSampleGrabberCB->SetBufferSamples(FALSE);
    pSampleGrabberCB->SetOneShot(FALSE);
    hr = pSampleGrabberCB->SetMediaType(mt);
    CHECK_HR(hr, "Can't add SampleGrabber to graph SetMediaType");

    //here we provide our callback:
    hr = pSampleGrabberCB->SetCallback(this, 1);
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

CComPtr<IBaseFilter> DShowCapture::GetFirstDevice(const IID &clsidDeviceClass, std::wstring &name) {
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
        name = varName.bstrVal;
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
    return S_OK;
    
    SampleTime *= 1000; 
    if (!pSample) {
        return E_POINTER;
    }
    long sz = pSample->GetActualDataLength();
    BYTE *pBuf = NULL;
    pSample->GetPointer(&pBuf);
    if (sz <= 0 || pBuf == NULL) {
        return E_UNEXPECTED;
    }

//     for (int i = 0; i < sz; i += 2) {
//         pBuf[i] = 255 - pBuf[i];
//     }
    //pSample->Release();

    unsigned char *buf = new unsigned char[sz];
    memcpy(buf, pBuf, sz);
    static int SampleTime1 = 0;
    if (m_pMediaDataCallback && m_captureType == euVideo) {
        m_pMediaDataCallback->OnVideoData(new DataBuffer(buf, sz, m_width, m_height, SampleTime1, false));
    } else if (m_pMediaDataCallback && m_captureType == euAudio) {
        m_pMediaDataCallback->OnAudioData(new DataBuffer(buf, sz, m_width, m_height, SampleTime1, false));
    }
    SampleTime1++;
    return S_OK;
}

STDMETHODIMP DShowCapture::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) {//声音
//     FILE *f1 = NULL;
//     fopen_s(&f1, "./test1.yuy2", "wb");
//     fwrite(pBuffer, BufferLen, 1, f1);
//     fclose(f1);
//     return S_OK;


    SampleTime *= 1000;
    LogI("SampleTime=%f\n", SampleTime);
    long sz = BufferLen;
    unsigned char *buf = new unsigned char[BufferLen];
    memcpy(buf, pBuffer, BufferLen);
    if (m_pMediaDataCallback && m_captureType == euVideo) {
        m_pMediaDataCallback->OnVideoData(new DataBuffer(buf, sz, m_width, m_height, SampleTime, false));
    }
    else if (m_pMediaDataCallback && m_captureType == euAudio) {
        //m_pMediaDataCallback->OnAudioData(new DataBuffer(buf, sz, m_width, m_height, SampleTime, false));
    }
    return S_OK;
}

/*For example, suppose that GetStreamCaps returns a 24-bit RGB format, with a frame size of 320 x 240 pixels. You can get this information by examining the major type, subtype, and format block of the media type:

if ((pmtConfig.majortype == MEDIATYPE_Video) &&
    (pmtConfig.subtype == MEDIASUBTYPE_RGB24) &&
    (pmtConfig.formattype == FORMAT_VideoInfo) &&
    (pmtConfig.cbFormat >= sizeof (VIDEOINFOHEADER)) &&
    (pmtConfig.pbFormat != NULL))
{
    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig.pbFormat;
    // pVih contains the detailed format information.
    LONG lWidth = pVih->bmiHeader.biWidth;
    LONG lHeight = pVih->bmiHeader.biHeight;
}*/
DShowCapture::~DShowCapture()
{
}
