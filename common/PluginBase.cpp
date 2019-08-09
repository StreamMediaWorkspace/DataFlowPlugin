#include "PluginBase.h"
#include "Logger.h"

PluginBase::PluginBase()
{
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
	for (int i = 0; i < (int)m_nextPluginVector.size(); i++){
		InputPluginBase *pInputPluginBase = (InputPluginBase*)m_nextPluginVector[i];
		assert(pInputPluginBase);
		pInputPluginBase->Input(data, len);
	}
}

void InputPluginBase::Connect(InputPluginBase *pNextOutputPlugin) {
	PluginBase::Connect((PluginBase*)pNextOutputPlugin);
}