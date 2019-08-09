#pragma once

#include "../common/PluginBase.h"
#include "DShowCapture.h"
#include <memory>

#define CAPTURE_PLUGIN "CapturePlugin"

class Capture :
	public OutputPluginBase,
    public MediaDataCallbackBase
{
public:
    Capture();
	~Capture();

	virtual const char* GetName(){
		return "capture";
	}

	virtual const char* GetVersion(){
		return "1.0.0.0";
	}

    virtual int Start();

    virtual void OnVideoData(void *data, int length);
    virtual void OnAudioData(void *data, int length);

private:
    std::shared_ptr<DShowCapture> m_pVideoCapture;
};

#ifndef CAPTURE_DLL_EXPORTS
#define CAPTURE_DLL_EXPORTS
#define CAPTURE_DLL_API __declspec(dllexport)
#else
#define CAPTURE_DLL_API __declspec(dllimport)
#endif

extern "C" CAPTURE_DLL_API OutputPluginBase* GetCaptureInstance();

#ifndef _LIB
#ifdef _WIN32
#include <windows.h>
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif

#endif

