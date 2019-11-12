#pragma once
#include "DShowCapture.h"
class DShowAudioCapture :
    public DShowCapture
{
public:
    DShowAudioCapture(MediaDataCallbackBase *pMediaDataCallback);
    ~DShowAudioCapture();

    virtual int Init();
    int SetAudioFormat(int samsPerSec, int bitsPerSam, int channels);

private:
    HRESULT Create();
    bool CheckStartParams();

    virtual void OnData(double SampleTime, BYTE *pBuffer, long BufferLen);

private:
    int m_samsPerSec = 0;
    int m_bitsPerSam = 0;
    int m_channels = 0;
};

