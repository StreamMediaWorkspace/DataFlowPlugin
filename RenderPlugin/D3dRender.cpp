#include "D3dRender.h"
#include "../common/Logger.h"

D3dRender::D3dRender()
{
}

int D3dRender::Start() {
    return PluginBase::Start();
}

int D3dRender::Stop() {
    return PluginBase::Stop();
}

void D3dRender::Input(DataBuffer *pDataBuffer){
	LogI("D3dRender::Input data=%p len=%p\n", pDataBuffer->Buf(), pDataBuffer->BufLen());
}

D3dRender::~D3dRender()
{
}

PluginBase* GetD3dRenderInstance()
{
    PluginBase* pInstance = new D3dRender();
	return pInstance;
}
