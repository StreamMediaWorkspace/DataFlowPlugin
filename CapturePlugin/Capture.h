#pragma once

#include "../common/PluginBase.h"
#include "DShowCapture.h"
#include <memory>

#define CAPTURE_PLUGIN "CapturePlugin"
#define CAPTURE_PLUGIN_VERSION "1.0.0.0"
#define CAPTURE_PLUGIN_KEY_WIDTH "width"
#define CAPTURE_PLUGIN_KEY_HEIGHT "height"
#define CAPTURE_PLUGIN_KEY_FPS "fps"

class Capture :
	public OutputPluginBase,
    public MediaDataCallbackBase
{
public:
    Capture();
	~Capture();

	virtual const char* GetName(){
		return CAPTURE_PLUGIN;
	}

	virtual const char* GetVersion(){
		return CAPTURE_PLUGIN_VERSION;
	}

    virtual int Start();

    virtual void OnVideoData(void *data, int length);
    virtual void OnAudioData(void *data, int length);

    virtual int Control(MetaData metaData);

private:
    std::shared_ptr<DShowCapture> m_pVideoCapture;

    int m_width = -1;
    int m_height = -1;
    int m_fps = -1;
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

