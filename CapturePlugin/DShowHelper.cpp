#include "DShowHelper.h"
#include "PinHelper.h"

#include "../common/Logger.h"

using namespace DShowHelper;

#define CHECK_HR(s) if (FAILED(s)) {return -1;}
void mediaSubtypeToText(const GUID guid, char *outText, int putMaxSize);
void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        // pUnk should not be used.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}
void _DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if (pmt != NULL)
    {
        _FreeMediaType(*pmt);
        CoTaskMemFree(pmt);
    }
}

int DShowHelper::EnumCaps(CComPtr<IBaseFilter> pSrc, CComPtr<ICaptureGraphBuilder2> pBuild)
{
    CComPtr<IAMStreamConfig> pConfig;
    try
    {
        //找到IAMStreamConfig接口
        CHECK_HR(pBuild->FindInterface(&PIN_CATEGORY_STILL, &MEDIATYPE_Video,
            pSrc, IID_IAMStreamConfig, (void**)&pConfig));

        //获取number数
        int iCnt, iSize;
        CHECK_HR(pConfig->GetNumberOfCapabilities(&iCnt, &iSize));

        //获取cps信息
        BYTE *pSCC = NULL;
        AM_MEDIA_TYPE *pmt;
        pSCC = new BYTE[iSize];
        if (pSCC == NULL) {
            LogE("error");
            return -1;
        }
        //遍历
        for (int i = 0; i < iCnt; i++)
        {
            char sz[64] = { 0 };
            CHECK_HR(pConfig->GetStreamCaps(i, &pmt, pSCC));	//获取信息
            mediaSubtypeToText(pmt->subtype, sz, sizeof(sz));
            //形成信息字符串，用于打印显示
            LogI("Cap index: %d \r\nMinOutputSize: %ld*%ld \r\nMaxOutputSize: %ld*%ld \r\nMinCroppingSize: %ld*%ld\r\nMaxCroppingSize: %ld*%ld\r\n majortype:%ld\r\nsubtype:",
                i,
                ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->MinOutputSize.cx,
                ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->MinOutputSize.cy,
                ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->MaxOutputSize.cx,
                ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->MaxOutputSize.cy,
                ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->MinCroppingSize.cx,
                ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->MinCroppingSize.cy,
                ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->MaxCroppingSize.cx,
                ((VIDEO_STREAM_CONFIG_CAPS*)pSCC)->MaxCroppingSize.cy,
                pmt->majortype);
            LogI("%s\r\n", sz);
           // DeleteMediaType(pmt);	//删除=============
        }

        delete[] pSCC;
    }
    catch (...)
    {
        MessageBox(NULL, _T("枚举Cap信息失败！"), _T(""), MB_OK);
        return -1;
    }

    return 0;
}

HRESULT DShowHelper::SetVideoFomat(int width, int height, int fps, CComPtr<IBaseFilter> pCameraSrc) {
    if (!pCameraSrc) {
        return -1;
    }

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
    format.bmiHeader.biWidth = width;
    format.bmiHeader.biHeight = height;
    format.bmiHeader.biPlanes = 1;
    format.bmiHeader.biBitCount = 16;
    format.bmiHeader.biCompression = 844715353;
    format.bmiHeader.biSizeImage = width*height;
    pmt.pbFormat = (BYTE*)&format;
    CComQIPtr<IAMStreamConfig, &IID_IAMStreamConfig> isc(PinHelper::GetPin(pCameraSrc, L"捕获"));
    return isc->SetFormat(&pmt);
}

void mediaSubtypeToText(const GUID guid, char *outText, int putMaxSize) {
    if (guid == MEDIASUBTYPE_RGB32) {
        strcpy_s(outText, putMaxSize, "MEDIASUBTYPE_RGB32");
    }
    else if (guid == MEDIASUBTYPE_RGB24) {
        strcpy_s(outText, putMaxSize, "MEDIASUBTYPE_RGB24");
    }
    else if (guid == MEDIASUBTYPE_RGB565) {
        strcpy_s(outText, putMaxSize, "MEDIASUBTYPE_RGB565");
    }
    else if (guid == MEDIASUBTYPE_YUY2) {
        strcpy_s(outText, putMaxSize, "MEDIASUBTYPE_YUY2");
    }
    else if (guid == MEDIASUBTYPE_UYVY) {
        strcpy_s(outText, putMaxSize, "MEDIASUBTYPE_UYVY");
    }
    else if (guid == MEDIASUBTYPE_YV12) {
        strcpy_s(outText, putMaxSize, "MEDIASUBTYPE_YV12");
    }
    else if (guid == MEDIASUBTYPE_MJPG) {
        strcpy_s(outText, putMaxSize, "MEDIASUBTYPE_MJPG");
    }
    else if (guid == MEDIASUBTYPE_NV12) {
        strcpy_s(outText, putMaxSize, "MEDIASUBTYPE_NV12");
    }
    else {
        strcpy_s(outText, putMaxSize, "unknown");
    }
}

/*IPin* DShowHelper::GetPin(IBaseFilter* filter, PIN_DIRECTION direction, int index)
{
    IPin* pin = NULL;

    CComPtr< IEnumPins > enum_pins;
    HRESULT hr = filter->EnumPins(&enum_pins);
    if (FAILED(hr))
    {
        return pin;
    }

    while (true)
    {
        ULONG ulFound;
        IPin *tmp_pin;
        hr = enum_pins->Next(1, &tmp_pin, &ulFound);
        if (hr != S_OK)
        {
            break;
        }

        PIN_DIRECTION pindir = (PIN_DIRECTION)3;
        tmp_pin->QueryDirection(&pindir);
        if (pindir == direction)
        {
            if (index == 0)
            {
                tmp_pin->AddRef();
                pin = tmp_pin;
                SAFE_RELEASE(tmp_pin);
                break;
            }
            index--;
        }

        SAFE_RELEASE(tmp_pin);
    }

    return pin;
}*/

HRESULT DShowHelper::SetAudioFormat(CComPtr<IBaseFilter> microphonefilter,
    DWORD samsPerSec,
    WORD bitsPerSam,
    WORD channels)
{
    CComQIPtr<IBaseFilter, &IID_IBaseFilter> cap_filter(microphonefilter);
    IAMBufferNegotiation *pNeg = NULL;
    WORD wBytesPerSample = bitsPerSam / 8;
    DWORD dwBytesPerSecond = wBytesPerSample * samsPerSec * channels;
    //DWORD dwBufferSize = (DWORD)(0.5*dwBytesPerSecond);   
    DWORD dwBufferSize;

    // setting buffer size according to the aac frame size   
    //     // (in narrow-band: 160*2 bytes)   
    //     switch( samsPerSec )   
    //     {   
    //     case 8000: { dwBufferSize = 320; break; }   
    //     case 11025: { dwBufferSize = 1280; break; } // AUDIO STREAM LAG DEPENDS ON THIS   
    //     case 22050: { dwBufferSize = 1280; break; }   
    //     case 44100: { dwBufferSize = 4096; break; }   
    //     default: dwBufferSize = 320;   
    //     }   

    // dwBufferSize = aac_frame_len * channels * wBytesPerSample   
    //              = 1024 * channels * wByesPerSample   
    dwBufferSize = 1024 * channels * wBytesPerSample;
    //dwBufferSize = 2048 * channels;   

    // get the nearest, or exact audio format the user wants   
    //   
    IEnumMediaTypes *pMedia = NULL;
    AM_MEDIA_TYPE *pmt = NULL;
    IPin* outPin = PinHelper::GetPin(cap_filter, L"Capture");
    HRESULT hr = outPin->EnumMediaTypes(&pMedia);

    if (SUCCEEDED(hr))
    {
        while (pMedia->Next(1, &pmt, 0) == S_OK)
        {
            if ((pmt->formattype == FORMAT_WaveFormatEx) &&
                (pmt->cbFormat == sizeof(WAVEFORMATEX)))
            {
                WAVEFORMATEX *wf = (WAVEFORMATEX *)pmt->pbFormat;

                if ((wf->nSamplesPerSec == samsPerSec) &&
                    (wf->wBitsPerSample == bitsPerSam) &&
                    (wf->nChannels == channels))
                {
                    // found correct audio format   
                    //   
                    CComPtr<IAMStreamConfig> pConfig;
                    hr = outPin->QueryInterface(IID_IAMStreamConfig, (void **)&pConfig);
                    if (SUCCEEDED(hr))
                    {
                        // get buffer negotiation interface   
                        outPin->QueryInterface(IID_IAMBufferNegotiation, (void **)&pNeg);

                        // set the buffer size based on selected settings   
                        ALLOCATOR_PROPERTIES prop = { 0 };
                        prop.cbBuffer = dwBufferSize;
                        prop.cBuffers = 6; // AUDIO STREAM LAG DEPENDS ON THIS   
                        prop.cbAlign = wBytesPerSample * channels;
                        pNeg->SuggestAllocatorProperties(&prop);
                        SAFE_RELEASE(pNeg);

                        WAVEFORMATEX *wf = (WAVEFORMATEX *)pmt->pbFormat;

                        LogI("nAvgBytesPerSec=%d\n", wf->nAvgBytesPerSec);
                        LogI("nBlockAlign=%d\n", wf->nBlockAlign);
                        LogI("nChannels=%d\n", wf->nChannels);
                        LogI("nSamplesPerSec=%d\n", wf->nSamplesPerSec);
                        LogI("wBitsPerSample=%d\n", wf->wBitsPerSample);

                        // setting additional audio parameters   
                        /*wf->nAvgBytesPerSec = dwBytesPerSecond;
                        wf->nBlockAlign = wBytesPerSample * channels;
                        wf->nChannels = channels;
                        wf->nSamplesPerSec = samsPerSec;
                        wf->wBitsPerSample = bitsPerSam;

                        pConfig->SetFormat(pmt);*/

                    }
                    else
                    {
                        pConfig.Release();
                        SAFE_RELEASE(pMedia);
                        _DeleteMediaType(pmt);
                        // can't set given audio format!   
                        return -1;
                    }

                    _DeleteMediaType(pmt);

                    hr = pConfig->GetFormat(&pmt);
                    if (SUCCEEDED(hr))
                    {
                        WAVEFORMATEX *wf = (WAVEFORMATEX *)pmt->pbFormat;

                        /*audio_cap_device_->SetSamplesPerSec( wf->nSamplesPerSec );
                        audio_cap_device_->SetBitsPerSample( wf->wBitsPerSample );
                        audio_cap_device_->SetChannels( wf->nChannels );*/

                        // audio is now initialized   
                        _DeleteMediaType(pmt);
                        pConfig.Release();
                        SAFE_RELEASE(pMedia);
                        return 0;
                    }

                    // error initializing audio   
                    _DeleteMediaType(pmt);
                    pConfig.Release();
                    SAFE_RELEASE(pMedia);
                    return -1;
                }
            }
            _DeleteMediaType(pmt);
        }
        SAFE_RELEASE(pMedia);
    }

    return -2;
}