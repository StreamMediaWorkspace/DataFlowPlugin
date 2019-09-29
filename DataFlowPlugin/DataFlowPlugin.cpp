// DataFlowPlugin.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../CapturePlugin/Capture.h"
#include "../RenderPlugin/D3dRender.h"
#include "../EncodePlugin/EncodePlugin.h"
#include "../RtmpPlugin/RtmpSenderPlugin.h"

#pragma comment(lib, "../Debug/CapturePlugin.lib")
#pragma comment(lib, "../Debug/RenderPlugin.lib")
#pragma comment(lib, "../Debug/EncodePlugin.lib")
#pragma comment(lib, "../Debug/RtmpPlugin.lib")

PluginBase *GetInstance(const std::string &name) {
    if (name == RENDER_PLUGIN) {
        return GetD3dRenderInstance();
    }
    else if (name == RTMP_SENDER_PLUGIN) {
        return GetRtmpSenderInstance();
    }
    else if (name == ENCODE_PLUGIN) {
        return GetEncodeInstance();
    } else if (name == CAPTURE_PLUGIN) {
        return GetCaptureInstance();
    }
    assert(false);
    return NULL;
}

int _tmain(int argc, _TCHAR* argv[])
{
    CoInitialize(NULL);
    PluginBase *capture = GetInstance(CAPTURE_PLUGIN);
    PluginBase *d3dRender = GetInstance(RENDER_PLUGIN);
    PluginBase *encoder = GetInstance(ENCODE_PLUGIN);
    PluginBase *rtmpSender = GetInstance(RTMP_SENDER_PLUGIN);

    int width = 640;
    int height = 480;
    int fps = 15;
    int bitrate = 25000;

    capture->Connect(d3dRender);
    capture->Connect(encoder);
    encoder->Connect(rtmpSender);

    MetaData madaData;
    madaData.setProperty(CAPTURE_PLUGIN_KEY_WIDTH, width);
    madaData.setProperty(CAPTURE_PLUGIN_KEY_HEIGHT, height);
    madaData.setProperty(CAPTURE_PLUGIN_KEY_FPS, fps);
    madaData.setProperty(ENCODE_PLUGIN_KEY_BITRATE, bitrate);
    capture->Control(madaData);

	capture->Start();
    while (1) {
        Sleep(1000);
    }
	system("pause");
    CoUninitialize();
	return 0;
}

