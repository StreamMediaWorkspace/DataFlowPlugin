// DataFlowPlugin.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../CapturePlugin/Capture.h"
#include "../RenderPlugin/D3dRender.h"

#pragma comment(lib, "../Debug/CapturePlugin.lib")
#pragma comment(lib, "../Debug/RenderPlugin.lib")

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
    assert(false);
    return NULL;
}

int _tmain(int argc, _TCHAR* argv[])
{
    CoInitialize(NULL);
	OutputPluginBase *capture = GetOutputInstance(CAPTURE_PLUGIN);
	InputPluginBase *d3dRender = GetInputInstance(RENDER_PLUGIN);
	capture->Connect(d3dRender);

	capture->Start();
    while (1) {
        Sleep(1000);
    }
	system("pause");
    CoUninitialize();
	return 0;
}

