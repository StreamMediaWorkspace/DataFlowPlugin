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

OutputPluginBase *GetOutputInstance(const std::string &name) {
    if (name == CAPTURE_PLUGIN) {
        return GetCaptureInstance();
    }

    assert(false);
    return NULL;
}

InputPluginBase *GetInputInstance(const std::string &name) {
    if (name == RENDER_PLUGIN) {
        return GetD3dRenderInstance();
    }
    else if (name == RTMP_SENDER_PLUGIN) {
        return GetRtmpSenderInstance();
    }
    else if (name == ENCODE_PLUGIN) {
        return GetEncodeInstance();
    }
    assert(false);
    return NULL;
}

int _tmain(int argc, _TCHAR* argv[])
{
    CoInitialize(NULL);
	OutputPluginBase *capture = GetOutputInstance(CAPTURE_PLUGIN);
	InputPluginBase *d3dRender = GetInputInstance(RENDER_PLUGIN);
    TransformPluginBase *encoder = (TransformPluginBase*)GetInputInstance(ENCODE_PLUGIN);
    InputPluginBase *rtmpSender = GetInputInstance(RTMP_SENDER_PLUGIN);

    int width = 320;
    int height = 240;
    int fps = 15;

    capture->Connect(d3dRender);
    capture->Connect(encoder);
    encoder->Connect(rtmpSender);

    MetaData madaData;
    madaData.setProperty(CAPTURE_PLUGIN_KEY_WIDTH, width);
    madaData.setProperty(CAPTURE_PLUGIN_KEY_HEIGHT, height);
    madaData.setProperty(CAPTURE_PLUGIN_KEY_FPS, fps);
    capture->Control(madaData);

	capture->Start();
    while (1) {
        Sleep(1000);
    }
	system("pause");
    CoUninitialize();
	return 0;
}

