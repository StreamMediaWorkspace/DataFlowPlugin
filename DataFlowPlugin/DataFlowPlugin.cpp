// DataFlowPlugin.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../CapturePlugin/Capture.h"
#include "../RenderPlugin/D3dRender.h"

#pragma comment(lib, "../Debug/CapturePlugin.lib")
#pragma comment(lib, "../Debug/RenderPlugin.lib")

OutputPluginBase *getOutputInstance(const std::string &name) {
    if (name == CAPTURE_PLUGIN) {
        return getCaptureInstance();
    }

    assert(false);
    return NULL;
}

InputPluginBase *getInputInstance(const std::string &name) {
    if (name == RENDER_PLUGIN) {
        return getD3dRenderInstance();
    }
    assert(false);
    return NULL;
}

int _tmain(int argc, _TCHAR* argv[])
{
	OutputPluginBase *capture = getOutputInstance(CAPTURE_PLUGIN);
	InputPluginBase *d3dRender = getInputInstance(RENDER_PLUGIN);
	capture->connect(d3dRender);

	capture->start();
	system("pause");
	return 0;
}

