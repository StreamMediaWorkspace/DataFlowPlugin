#include "D3dRender.h"
#include "../common/Logger.h"

D3dRender::D3dRender()
{
}


void D3dRender::Input(const void *data, int len){
	LogI("D3dRender::Input data=%p len=%d\n", data, len);
}

D3dRender::~D3dRender()
{
}

InputPluginBase* GetD3dRenderInstance()
{
	InputPluginBase* pInstance = new D3dRender();
	return pInstance;
}
