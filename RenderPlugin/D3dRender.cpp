#include "D3dRender.h"


D3dRender::D3dRender()
{
}


void D3dRender::input(const void *data, int len){
	LOG_FUNCTION;
	printf("%s\n", (char*)data);
}

D3dRender::~D3dRender()
{
}

InputPluginBase* getD3dRenderInstance()
{
	LOG_FUNCTION;
	InputPluginBase* pInstance = new D3dRender();
	return pInstance;
}
