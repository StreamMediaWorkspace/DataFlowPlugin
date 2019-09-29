#pragma once
#include <stdio.h>

#include "../common/PluginBase.h"
#include "../common/Logger.h"

#define RTMP_SENDER_PLUGIN "RtmpSender"
#define RTMP_SENDER_PLUGIN_VERSION "1.0.0.1"

#include "RtmpSender.h"
class RtmpSenderPlugin : public PluginBase
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

    virtual int Start();
    virtual int Stop();
    virtual void Input(DataBuffer *pDataBuffer);

private:
    static void SenderThread(RtmpSenderPlugin* pRtmpSenderPlugin);
    void Loop();

    void UpdatePPS(char *pps, int ppsSize);
    void UpdateSPS(char *sps, int spsSize);
    int MetaDataToBuffer(int width, int height, char **outBuffer);
    int SpsPpsToBuffer(char **outBuffer);
    int BodyToBuffer(char *nalData, int size, bool isKeyFrame, char **outBuffer);
private:
    RtmpSender m_rtmpSender;
    DataBufferQueue m_dataBufferQueue;
    bool m_stop = false;
    std::thread *m_pThread = NULL;

    char *m_sps = NULL;
    int m_spsSize = 0;
    char *m_pps = NULL;
    int m_ppsSize = 0;
};

#ifndef RTMP_SENDER_DLL_EXPORTS
#define RTMP_SENDER_DLL_EXPORTS
#define RTMP_SENDER_DLL_API __declspec(dllexport)
#else
#define RTMP_SENDER_DLL_API __declspec(dllimport)
#endif

extern "C" RTMP_SENDER_DLL_API PluginBase* GetRtmpSenderInstance();

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


