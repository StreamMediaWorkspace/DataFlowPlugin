#include "PluginBase.h"
#include "Logger.h"


PluginBase::PluginBase() {
}

int PluginBase::Start() {
    for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
        PluginBase *pPluginBase = (PluginBase*)m_nextPluginVector[i];
        assert(pPluginBase);
        pPluginBase->Start();
    }
    return 0;
}

int PluginBase::Stop() {
    for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
        PluginBase *pPluginBase = (PluginBase*)m_nextPluginVector[i];
        assert(pPluginBase);
        pPluginBase->Stop();
    }
    return 0;
}

int PluginBase::Control(MetaData madaData) {
    if (m_nextPluginVector.size() == 0) {
        LogI("PluginBase::Control %s\n", __FUNCTION__);
    }

    for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
        PluginBase *pInputPluginBase = (PluginBase*)m_nextPluginVector[i];
        assert(pInputPluginBase);
        pInputPluginBase->Control(madaData);
    }
    return 0;
}

void PluginBase::Connect(PluginBase *pNextPluginBase) {
	assert(pNextPluginBase);
	LogI("%s->%s\n", GetName(), pNextPluginBase->GetName());
	m_nextPluginVector.push_back(pNextPluginBase);
}

void PluginBase::Input(DataBuffer *pdataBuffer) {
    for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
        PluginBase *pInputPluginBase = (PluginBase*)m_nextPluginVector[i];
        assert(pInputPluginBase);
        pInputPluginBase->Input(pdataBuffer);
    }
}

PluginBase::~PluginBase()
{
}

// 
// OutputPluginBase::OutputPluginBase() {
// }
// 
// OutputPluginBase::~OutputPluginBase() {
// }
// 
// void OutputPluginBase::Connect(InputPluginBase *pNextInputPlugin) {
// 	PluginBase::Connect((PluginBase*)pNextInputPlugin);
// }
// 
// void OutputPluginBase::Output(DataBuffer *pDataBuffer) {
// 	for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
// 		InputPluginBase *pInputPluginBase = (InputPluginBase*)m_nextPluginVector[i];
// 		assert(pInputPluginBase);
// 		pInputPluginBase->Input(pDataBuffer);
// 	}
// }
// 
// void InputPluginBase::Connect(InputPluginBase *pNextOutputPlugin) {
// 	PluginBase::Connect((PluginBase*)pNextOutputPlugin);
// }
// 
// void TransformPluginBase::Output(DataBuffer *pDataBuffer) {
//     for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
//         InputPluginBase *pInputPluginBase = (InputPluginBase*)m_nextPluginVector[i];
//         assert(pInputPluginBase);
//         InputPluginBase::Input(pDataBuffer);
//     }
// }