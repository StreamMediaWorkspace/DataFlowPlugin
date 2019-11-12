#pragma once
#include <strmif.h>  
#include <atlbase.h>
#include <uuids.h>
#include <list>

#define SAFE_RELEASE(p)
struct VideoFormat {
    int width;
    int height;
    int frameRate;

    union {
        char fourccName[8];
        unsigned int fourcc;
    };

    int bitCount;
};
bool operator==(const VideoFormat& cThs, const VideoFormat& other);
bool operator<(const VideoFormat& cThs, const VideoFormat& other);
void _FreeMediaType(AM_MEDIA_TYPE& mt);
void _DeleteMediaType(AM_MEDIA_TYPE *pmt);

class DShowHelper
{
public:
    int EnumCaps(CComPtr<IBaseFilter> pSrc, CComPtr<ICaptureGraphBuilder2> pBuild);
    HRESULT SetAudioFormat(CComPtr<IBaseFilter> microphonefilter,
        DWORD samsPerSec,
        WORD bitsPerSam,
        WORD channels);

    static HRESULT SetVideoFomat(int width, int height, int fps, CComPtr<IBaseFilter> pCameraSrc);
    static bool GetCameraSupportedResolution(std::list<VideoFormat>& frameRates, CComPtr<IBaseFilter> pCameraSrc);
    static bool SetCameraSupportedResolution(int index, CComPtr<IBaseFilter> pCameraSrc, CComPtr<ICaptureGraphBuilder2> builder2);

private:
    template<class T>
    static VideoFormat& VideoFormatCast(VideoFormat& format, T  h) {
        format.fourccName[4] = 0;
        format.width = h->bmiHeader.biWidth;
        format.height = h->bmiHeader.biHeight;
        format.frameRate = h->AvgTimePerFrame ? (int)(10000000LL / h->AvgTimePerFrame) : 30;
        format.bitCount = h->bmiHeader.biBitCount;
        if (h->bmiHeader.biCompression == 0)
            format.fourcc = 0;
        return format;
    }

    static void DeleteMediaType(AM_MEDIA_TYPE *pmt)
    {
        if (pmt != NULL)
        {
            _FreeMediaType(*pmt);
            CoTaskMemFree(pmt);
        }
    }

    static void _FreeMediaType(AM_MEDIA_TYPE& mt)
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
};

