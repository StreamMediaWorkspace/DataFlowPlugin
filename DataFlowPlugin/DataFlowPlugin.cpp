// DataFlowPlugin.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../CapturePlugin/Capture.h"
#include "../RenderPlugin/D3dRender.h"

#pragma comment(lib, "F:\\c_plus_plus\\DataFlowPlugin\\Debug\\CapturePlugin.lib")
#pragma comment(lib, "F:\\c_plus_plus\\DataFlowPlugin\\Debug\\RenderPlugin.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	OutputPluginBase *capture = getCaptureInstance();
	InputPluginBase *d3dRender = getD3dRenderInstance();
	capture->connect(d3dRender);

	capture->start();
	system("pause");
	return 0;
}

