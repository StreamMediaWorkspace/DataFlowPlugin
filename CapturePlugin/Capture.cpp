#include "Capture.h"


Capture::Capture()
{
}

Capture::~Capture()
{
}

OutputPluginBase* getCaptureInstance()
{
	LOG_FUNCTION;
	OutputPluginBase* pInstance = new Capture();
	return pInstance;
}
