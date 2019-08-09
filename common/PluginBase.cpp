#include "PluginBase.h"

PluginBase::PluginBase()
{
	LOG_FUNCTION;
}

void PluginBase::Connect(PluginBase *pNextPluginBase) {
	LOG_FUNCTION;
	assert(pNextPluginBase);
	printf("%s->%s\n", GetName(), pNextPluginBase->GetName());
	m_nextPluginVector.push_back(pNextPluginBase);
}

PluginBase::~PluginBase()
{
	LOG_FUNCTION;
}


OutputPluginBase::OutputPluginBase() {
	LOG_FUNCTION;
}

OutputPluginBase::~OutputPluginBase() {
	LOG_FUNCTION;
}

void OutputPluginBase::Connect(InputPluginBase *pNextInputPlugin) {
	LOG_FUNCTION;
	PluginBase::Connect((PluginBase*)pNextInputPlugin);
}

void OutputPluginBase::Output(const void * data, int len) {
	LOG_FUNCTION;
	for (int i = 0; i < (int)m_nextPluginVector.size(); i++){
		InputPluginBase *pInputPluginBase = (InputPluginBase*)m_nextPluginVector[i];
		assert(pInputPluginBase);
		pInputPluginBase->Input(data, len);
	}
}

void InputPluginBase::Connect(InputPluginBase *pNextOutputPlugin) {
	LOG_FUNCTION;
	PluginBase::Connect((PluginBase*)pNextOutputPlugin);
}