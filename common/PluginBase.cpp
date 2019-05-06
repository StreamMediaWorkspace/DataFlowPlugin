#include "PluginBase.h"

PluginBase::PluginBase()
{
	LOG_FUNCTION;
}


void PluginBase::connect(PluginBase *pNextPluginBase) {
	LOG_FUNCTION;
	assert(pNextPluginBase);
	printf("%s->%s\n", getName(), pNextPluginBase->getName());
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

void OutputPluginBase::connect(InputPluginBase *pNextInputPlugin) {
	LOG_FUNCTION;
	PluginBase::connect((PluginBase*)pNextInputPlugin);
}

void OutputPluginBase::output(const void * data, int len) {
	LOG_FUNCTION;
	for (int i = 0; i < (int)m_nextPluginVector.size(); i++){
		InputPluginBase *pInputPluginBase = (InputPluginBase*)m_nextPluginVector[i];
		assert(pInputPluginBase);
		pInputPluginBase->input(data, len);
	}
}


void InputPluginBase::connect(InputPluginBase *pNextOutputPlugin) {
	LOG_FUNCTION;
	PluginBase::connect((PluginBase*)pNextOutputPlugin);
}