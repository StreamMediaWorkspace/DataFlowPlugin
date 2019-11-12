#include "DShowAudioCapture.h"
#include "../common/Logger.h"

DShowAudioCapture::DShowAudioCapture(MediaDataCallbackBase *pMediaDataCallback)
    : DShowCapture(pMediaDataCallback) {
}

int DShowAudioCapture::Init() {
    HRESULT hr = Create();
    if (!SUCCEEDED(hr)) {
        LogE("error: DShowAudioCapture start BuildGraph %ld\n", hr);
        return -1;
    }
    return 0;
}

HRESULT DShowAudioCapture::Create() {
    if (false == CheckStartParams()) {
        LogE("SetCapDevice CheckStartParams\n");
        return -1;
    }

    if (!SetCapDevice(CLSID_AudioInputDeviceCategory, m_deviceID)) {
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

    hr = m_pGraph->AddFilter(m_pDeviceFilter, L"Source");
    CHECK_HR(hr, "Can't add capture to graph");

    //add video SampleGrabber
    CComPtr<IBaseFilter> pSampleGrabber;
    hr = pSampleGrabber.CoCreateInstance(CLSID_SampleGrabber);
    CHECK_HR(hr, "Can't create video SampleGrabber");
    hr = m_pGraph->AddFilter(pSampleGrabber, L"SampleGrabber");
    CHECK_HR(hr, "Can't add SampleGrabber to graph");
    CComQIPtr<ISampleGrabber, &IID_ISampleGrabber> pSampleGrabberCB(pSampleGrabber);

    if (-1 == SetAudioFormat(44100, 16, 2)) {
        LogE("SetAudioFormat error\n");
        return -1;
    }

    //here we provide our callback:
    hr = pSampleGrabberCB->SetCallback(this, 1);
    CHECK_HR(hr, "Can't set callback");

    hr = m_pBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, m_pDeviceFilter, NULL, pSampleGrabber);
    CHECK_HR(hr, _T("Can't render stream to SampleGrabber"));
}

bool DShowAudioCapture::CheckStartParams() {
    return true;
}

void DShowAudioCapture::OnData(double SampleTime, BYTE *pBuffer, long BufferLen) {
    LogI("SampleTime=%f\n", SampleTime);
    long sz = BufferLen;
    unsigned char *buf = new unsigned char[BufferLen];
    memcpy(buf, pBuffer, BufferLen);
    if (m_pMediaDataCallback) {
        m_pMediaDataCallback->OnData(new AudioDataBuffer(buf, sz, m_samsPerSec, m_bitsPerSam, m_channels, SampleTime));
    }
}

int DShowAudioCapture::SetAudioFormat(int samsPerSec, int bitsPerSam, int channels) {
    if (!samsPerSec || !bitsPerSam || !channels) {
        LogE("SetAudioFormat failed\n");
        return -1;
    }
    m_samsPerSec = samsPerSec;
    m_bitsPerSam = bitsPerSam;
    m_channels = channels;

    if (!m_pBuilder2) {
        return -1;
    }

    HRESULT hRet;
    WORD wBytesPerSample = bitsPerSam / 8;
    DWORD dwBytesPerSecond = wBytesPerSample * samsPerSec * channels;
    DWORD dwBufferSize = 1024 * channels * wBytesPerSample;

    IAMStreamConfig *pConfig = NULL;
    if (FAILED(hRet = m_pBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
        m_pDeviceFilter, IID_IAMStreamConfig, (void**)&pConfig))) {
        return -1;
    }

    IAMBufferNegotiation *pNeg;
    if (FAILED(hRet = m_pBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
        m_pDeviceFilter, IID_IAMBufferNegotiation, (void**)&pNeg))) {
        return -1;
    }

    int iCount = 0, iSize = 0;
    if (FAILED(pConfig->GetNumberOfCapabilities(&iCount, &iSize)))
        return -1;

    if (iSize == sizeof(AUDIO_STREAM_CONFIG_CAPS)) {
        for (int iFormat = 0; iFormat < iCount; iFormat++) {
            AUDIO_STREAM_CONFIG_CAPS scc;
            AM_MEDIA_TYPE *pmtConfig;
            HRESULT hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);

            if (SUCCEEDED(hr)) {
                if ((pmtConfig->majortype == MEDIATYPE_Audio) &&
                    (pmtConfig->formattype == FORMAT_WaveFormatEx) &&
                    (pmtConfig->cbFormat >= sizeof(WAVEFORMATEX)) &&
                    (pmtConfig->pbFormat != NULL)) {
                    WAVEFORMATEX *wf = (WAVEFORMATEX *)pmtConfig->pbFormat;
                    if ((wf->nSamplesPerSec == samsPerSec) &&
                        (wf->wBitsPerSample == bitsPerSam) &&
                        (wf->nChannels == channels)) {
                        ALLOCATOR_PROPERTIES prop = { 0 };
                        prop.cbBuffer = dwBufferSize;
                        prop.cBuffers = 6;
                        prop.cbAlign = wBytesPerSample * channels;
                        pNeg->SuggestAllocatorProperties(&prop);

                        WAVEFORMATEX *wf = (WAVEFORMATEX *)pmtConfig->pbFormat;
                        wf->nAvgBytesPerSec = dwBytesPerSecond;
                        wf->nBlockAlign = wBytesPerSample * channels;
                        wf->nChannels = channels;
                        wf->nSamplesPerSec = samsPerSec;
                        wf->wBitsPerSample = bitsPerSam;

                        pConfig->SetFormat(pmtConfig);
                    }
                }

                _DeleteMediaType(pmtConfig);
            }
        }
    }

    SAFE_RELEASE(pConfig);
    SAFE_RELEASE(pNeg);

    return 0;
}

DShowAudioCapture::~DShowAudioCapture()
{
}
