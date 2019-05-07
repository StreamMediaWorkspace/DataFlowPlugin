#pragma once
#include "../common/PluginBase.h"
#define CAPTURE_PLUGIN "CapturePlugin"
class Capture :
	public OutputPluginBase
{
public:
	Capture();
	~Capture();

	virtual const char* getName(){
		return "capture";
	}

	virtual const char* getVersion(){
		return "1.0.0.0";
	}

	virtual int start() {
		LOG_FUNCTION;
		std::string s = "hello world";
		OutputPluginBase::output(s.c_str(), s.length());

		return 0;
	}
};

#ifndef CAPTURE_DLL_EXPORTS
#define CAPTURE_DLL_EXPORTS
#define CAPTURE_DLL_API __declspec(dllexport)
#else
#define CAPTURE_DLL_API __declspec(dllimport)
#endif

extern "C" CAPTURE_DLL_API OutputPluginBase* getCaptureInstance();

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

