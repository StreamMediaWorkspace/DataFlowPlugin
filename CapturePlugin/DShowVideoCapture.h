#pragma once
#include "DShowCapture.h"

class DShowVideoCapture : public DShowCapture {
public:
    DShowVideoCapture(MediaDataCallbackBase *pMediaDataCallback);
    int SetVideoFormat(int width, int height, int fps);
    virtual int Init();
    
    bool GetCameraSupportedResolution(std::list<VideoFormat>& frameRates);
    bool SetCameraSupportedResolution(int index);

    ~DShowVideoCapture();
    
private:
    HRESULT Create();
    bool CheckStartParams();
    virtual void OnData(double SampleTime, BYTE *pBuffer, long BufferLen);

private:
    int m_width = 0;
    int m_height = 0;
    int m_fps = 0;

    AM_MEDIA_TYPE *m_mediaType = nullptr;
};