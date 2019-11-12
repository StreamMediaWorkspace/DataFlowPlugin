#include "DShowVideoCapture.h"
#include <qedit.h>
#include <uuids.h>
#include <amvideo.h>
#include "DShowHelper.h"
#include "PinHelper.h"
#include "../common/Logger.h"
#include "../common/Utils.h"
#include "../common/PluginBase.h"

#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib,"Quartz.lib")

DShowVideoCapture::DShowVideoCapture(MediaDataCallbackBase *pMediaDataCallback)
    :DShowCapture(pMediaDataCallback){
}

int DShowVideoCapture::Init() {
    HRESULT hr = Create();
    if (!SUCCEEDED(hr)) {
        LogE("error: start BuildGraph %ld\n", hr);
        return -1;
    }
    return 0;
}

void DShowVideoCapture::OnData(double SampleTime, BYTE *pBuffer, long BufferLen) {
    LogI("SampleTime=%f\n", SampleTime);
    long sz = BufferLen;
    unsigned char *buf = new unsigned char[BufferLen];
    memcpy(buf, pBuffer, BufferLen);
    if (m_pMediaDataCallback) {
        m_pMediaDataCallback->OnData(new VideoDataBuffer(buf, sz, m_width, m_height, SampleTime));
    }
}

bool DShowVideoCapture::GetCameraSupportedResolution(std::list<VideoFormat>& frameRates) {
    return DShowHelper::GetCameraSupportedResolution(frameRates, m_pDeviceFilter);
}

bool DShowVideoCapture::SetCameraSupportedResolution(int index) {
    if (!m_pDeviceFilter || !m_pBuilder2) {
        LogE("DShowCapture SetCameraSupportedResolution %p,%p\n", m_pDeviceFilter, m_pBuilder2);
        return false;
    }
    return DShowHelper::SetCameraSupportedResolution(index, m_pDeviceFilter, m_pBuilder2);
}

bool DShowVideoCapture::CheckStartParams() {
    if (m_deviceID.empty()) {
        return false;
    }
    if (!m_width || !m_height || !m_fps) {
        LogE("SetVideoFormat error\n");
        return false;
    }

    return true;

}

HRESULT DShowVideoCapture::Create() {
    if (false == CheckStartParams()) {
        LogE("SetCapDevice CheckStartParams\n");
        return -1;
    }

    if (!SetCapDevice(CLSID_VideoInputDeviceCategory, m_deviceID)) {
        LogE("SetCapDevice error\n");
        return -1;
    }

    HRESULT hr = m_pGraph.CoCreateInstance(CLSID_FilterGraph);
    CHECK_HR(hr, "create filter graph error");
    LogI("Building graph...\n");

    hr = m_pBuilder2.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    CHECK_HR(hr, _T("Can't create Capture Graph Builder"));
    hr = m_pBuilder2->SetFiltergraph(m_pGraph);
    CHECK_HR(hr, "Can't SetFiltergraph");

    //add USB2.0 Camera
    //CComPtr<IBaseFilter> pUSB20Camera = CreateFilterByName(L"USB2.0 Camera", CLSID_VideoCaptureSources);
//    std::wstring name;

    //m_pDeviceFilter = GetFirstDevice(CLSID_VideoInputDeviceCategory, name);
//     } else {
//         m_pSrcFilter = GetFirstDevice(CLSID_AudioInputDeviceCategory, name);
//     }

    //hr = m_pGraph->AddFilter(m_pDeviceFilter, name.c_str()/*L"USB2.0 Camera"*//*L"Integrated Camera"*//*L"TTQ_HD_1080P Camera"*/);
    hr = m_pGraph->AddFilter(m_pDeviceFilter, L"Source");
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

    //设置视频分辨率、格式 
//     IAMStreamConfig *pConfig = NULL;
//     m_pBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
//         m_pDeviceFilter, IID_IAMStreamConfig, (void **)&pConfig);

    if (-1 == SetVideoFormat(m_width, m_height, m_fps)) {
        LogE("SetVideoFormat error\n");
        return -1;
    }

    if (!m_mediaType) {
        LogE("m_mediaType is null!!!!!!!!!!!!!\n");
        return -1;
    }

    //是否缓存数据，缓存的话，可以给后面做其他处理，不缓存的话，图像处理就放在回调中
    pSampleGrabberCB->SetBufferSamples(FALSE);
    pSampleGrabberCB->SetOneShot(FALSE);
    hr = pSampleGrabberCB->SetMediaType(m_mediaType);
    CHECK_HR(hr, "Can't add SampleGrabber to graph SetMediaType");

    //here we provide our callback:
    hr = pSampleGrabberCB->SetCallback(this, 1);
    CHECK_HR(hr, "Can't set callback");

    hr = m_pBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pDeviceFilter, NULL, pSampleGrabber);
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

int DShowVideoCapture::SetVideoFormat(int width, int height, int fps) {
    if (!width || !height || !fps) {
        LogE("SetVideoFormat error\n");
        return -1;
    }

    m_width = width;
    m_height = height;
    m_fps = fps;

    if (!m_pBuilder2) {
        return -1;
    }

    IAMStreamConfig *pConfig = NULL;
    HRESULT hRet;
    if (FAILED(hRet = m_pBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
        m_pDeviceFilter, IID_IAMStreamConfig, (void**)&pConfig)))
    {
        return -1;
    }

    int iCount = 0, iSize = 0;
    if (FAILED(pConfig->GetNumberOfCapabilities(&iCount, &iSize))) {
        return -1;
    }

    bool find_fmt = false;
    if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)) {
        for (int iFormat = 0; iFormat < iCount; iFormat++) {
            VIDEO_STREAM_CONFIG_CAPS scc;
            AM_MEDIA_TYPE *pmtConfig;
            HRESULT hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);

            if (SUCCEEDED(hr)) {
                if ((pmtConfig->majortype == MEDIATYPE_Video) &&
                    /*(pmtConfig->subtype == MEDIASUBTYPE_RGB24) &&*/
                    (pmtConfig->formattype == FORMAT_VideoInfo) &&
                    (pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
                    (pmtConfig->pbFormat != NULL)) {
                    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;

                    LONG lWidth = pVih->bmiHeader.biWidth;
                    LONG lHeight = pVih->bmiHeader.biHeight;

                    if ((lWidth == width) && (lHeight == height)) {
                        find_fmt = true;

                        pVih->AvgTimePerFrame = 10000000 / fps;
                        hr = pConfig->SetFormat(pmtConfig);
                        m_mediaType = pmtConfig;
                        break;
                    }
                }

                _DeleteMediaType(pmtConfig);
            }
        }
    }

    SAFE_RELEASE(pConfig);

    return 0;
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
DShowVideoCapture::~DShowVideoCapture()
{
}
