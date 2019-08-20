#pragma once
#include <stdio.h>

#include "../common/PluginBase.h"
#include "../common/Logger.h"

#define RTMP_SENDER_PLUGIN "RtmpSender"
#define RTMP_SENDER_PLUGIN_VERSION "1.0.0.1"

#include "RtmpSender.h"
class RtmpSenderPlugin : public InputPluginBase
{
public:
    RtmpSenderPlugin();
    ~RtmpSenderPlugin();

    virtual const char* GetName() {
        return RTMP_SENDER_PLUGIN;
    }

    virtual const char* GetVersion() {
        return RTMP_SENDER_PLUGIN_VERSION;
    }

    virtual void Input(const void * data, int len);

private:
    RtmpSender m_rtmpSender;
};

#ifndef RTMP_SENDER_DLL_EXPORTS
#define RTMP_SENDER_DLL_EXPORTS
#define RTMP_SENDER_DLL_API __declspec(dllexport)
#else
#define RTMP_SENDER_DLL_API __declspec(dllimport)
#endif

extern "C" RTMP_SENDER_DLL_API InputPluginBase* GetRtmpSenderInstance();

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


