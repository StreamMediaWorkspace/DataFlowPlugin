#pragma once
#include "../common/PluginBase.h"
#include "X246Encode.h"
#define ENCODE_PLUGIN "EncodePlugin"

#define ENCODE_PLUGIN_KEY_WIDTH "width"
#define ENCODE_PLUGIN_KEY_HEIGHT "height"
#define ENCODE_PLUGIN_KEY_FPS "fps"

class EncodePlugin : public TransformPluginBase {
public:
    EncodePlugin();
    ~EncodePlugin();

    virtual const char* GetName() {
        return ENCODE_PLUGIN;
    }

    virtual const char* GetVersion() {
        return "1.0.0.0";
    }

    virtual int Control(MetaData metaData);

protected:
    virtual void Input(const void * data, int len);

private:
    X246Encode m_X246Encode;

    int m_nWidth = -1;
    int m_nHeight = -1;
    int m_nFps = -1;
};

#ifndef ENCODE_DLL_EXPORTS
#define ENCODE_DLL_EXPORTS
#define ENCODE_DLL_API __declspec(dllexport)
#else
#define ENCODE_DLL_EXPORTS __declspec(dllimport)
#endif

extern "C" ENCODE_DLL_EXPORTS TransformPluginBase* GetEncodeInstance();

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
