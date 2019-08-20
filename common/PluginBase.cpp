#include "PluginBase.h"
#include "Logger.h"


PluginBase::PluginBase()
{
}

int PluginBase::Control(MetaData madaData) {
    if (m_nextPluginVector.size() == 0) {
        LogI("PluginBase::Control %s\n", __FUNCTION__);
    }

    for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
        InputPluginBase *pInputPluginBase = (InputPluginBase*)m_nextPluginVector[i];
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

PluginBase::~PluginBase()
{
}


OutputPluginBase::OutputPluginBase() {
}

OutputPluginBase::~OutputPluginBase() {
}

void OutputPluginBase::Connect(InputPluginBase *pNextInputPlugin) {
	PluginBase::Connect((PluginBase*)pNextInputPlugin);
}

void OutputPluginBase::Output(const void * data, int len) {
	for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
		InputPluginBase *pInputPluginBase = (InputPluginBase*)m_nextPluginVector[i];
		assert(pInputPluginBase);
		pInputPluginBase->Input(data, len);
	}
}

void InputPluginBase::Connect(InputPluginBase *pNextOutputPlugin) {
	PluginBase::Connect((PluginBase*)pNextOutputPlugin);
}

void TransformPluginBase::Output(const void * data, int len) {
    for (int i = 0; i < (int)m_nextPluginVector.size(); i++) {
        InputPluginBase *pInputPluginBase = (InputPluginBase*)m_nextPluginVector[i];
        assert(pInputPluginBase);
        pInputPluginBase->Input(data, len);
    }
}