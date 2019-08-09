#pragma once
#include "../common/PluginBase.h"
#define RENDER_PLUGIN "RenderPlugin"
class D3dRender :
	public InputPluginBase
{
public:
	D3dRender();
	~D3dRender();

	virtual const char* GetName(){
		return "D3Drender";
	}

	virtual const char* GetVersion(){
		return "1.0.0.0";
	}

	virtual void Input(const void *data, int len);
};

#ifndef D3DRENDER_DLL_EXPORTS
#define D3DRENDER_DLL_EXPORTS
#define D3DRENDER_DLL_API __declspec(dllexport)
#else
#define D3DRENDER_DLL_API __declspec(dllimport)
#endif

extern "C" D3DRENDER_DLL_API InputPluginBase* GetD3dRenderInstance();

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