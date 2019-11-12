#pragma once
#include "stdafx.h"
#include "../common/PluginBase.h"
#include "X264Encoder.h"
#include "FAACEncoder.h"

#define ENCODE_PLUGIN "EncodePlugin"
#define ENCODE_PLUGIN_KEY_VERSION "1.0.0.0"

#define ENCODE_PLUGIN_KEY_WIDTH "width"
#define ENCODE_PLUGIN_KEY_HEIGHT "height"
#define ENCODE_PLUGIN_KEY_FPS "fps"
#define ENCODE_PLUGIN_KEY_BITRATE "bitrate"

class EncodePlugin : public PluginBase {
public:
    EncodePlugin();
    ~EncodePlugin();

    virtual const char* GetName() {
        return ENCODE_PLUGIN;
    }

    virtual const char* GetVersion() {
        return ENCODE_PLUGIN_KEY_VERSION;
    }

    virtual int Start();
    virtual int Stop();
    virtual int Control(MetaData metaData);

protected:
    virtual void Input(DataBuffer *pDataBuffer);

private:
    void SaveRgb2Bmp(char* rgbbuf, unsigned int width, unsigned int height);
    void Loop();
    static void EncodeThread(EncodePlugin *pEncodePlugin);
    
private:
    X264Encoder m_X246Encode;
    FAACEncoder m_faacEncode;
    std::thread *m_pThread = NULL;

    int m_nWidth = -1;
    int m_nHeight = -1;
    int m_nFps = -1;
    int m_nBitrate = -1;

    unsigned int m_sampleRate = 44100;
    unsigned int m_channels = 2;
    unsigned int m_bitsPerSample = 16;

    DataBufferQueue m_dataBufferQueue;
    bool m_stop = false;

    unsigned char *m_sps = NULL;
    int m_spsLength = 0;

    unsigned char *m_pps = NULL;
    int m_ppsLength = 0;

    int m_index = 0;

    char *m_accBufferToEncode = nullptr;
};

#ifndef ENCODE_DLL_EXPORTS
#define ENCODE_DLL_EXPORTS
#define ENCODE_DLL_API __declspec(dllexport)
#else
#define ENCODE_DLL_EXPORTS __declspec(dllimport)
#endif

extern "C" ENCODE_DLL_EXPORTS PluginBase* GetEncodeInstance();

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
