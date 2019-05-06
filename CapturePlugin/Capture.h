#pragma once
#include "../common/PluginBase.h"
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

