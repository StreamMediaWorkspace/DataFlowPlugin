#pragma once
#include <strmif.h>  
#include <atlbase.h>
#include <uuids.h>
#define SAFE_RELEASE(p)
/*#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) \
  if ((p)) {            \
    delete (p);         \
    (p) = NULL;         \
  }
#endif*/

namespace DShowHelper
{
    int EnumCaps(CComPtr<IBaseFilter> pSrc, CComPtr<ICaptureGraphBuilder2> pBuild);
    HRESULT SetAudioFormat(CComPtr<IBaseFilter> microphonefilter,
        DWORD samsPerSec,
        WORD bitsPerSam,
        WORD channels);
};

