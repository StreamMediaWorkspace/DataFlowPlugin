#pragma once
#include "../common/PluginBase.h"
class D3dRender :
	public InputPluginBase
{
public:
	D3dRender();
	~D3dRender();

	virtual const char* getName(){
		return "D3Drender";
	}

	virtual const char* getVersion(){
		return "1.0.0.0";
	}

	virtual void input(const void *data, int len);
};

#ifndef D3DRENDER_DLL_EXPORTS
#define D3DRENDER_DLL_EXPORTS
#define D3DRENDER_DLL_API __declspec(dllexport)
#else
#define D3DRENDER_DLL_API __declspec(dllimport)
#endif

extern "C" D3DRENDER_DLL_API InputPluginBase* getD3dRenderInstance();
