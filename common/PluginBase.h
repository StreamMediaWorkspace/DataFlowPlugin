#pragma once
#include <vector>
#include <assert.h>

#define LOG_FUNCTION printf("%s\n", __FUNCTION__);

//#define LIB

class PluginBase
{
public:
	PluginBase();
	~PluginBase();
	
	virtual const char* getName() = 0;
	virtual const char* getVersion() = 0;

protected:
	virtual void connect(PluginBase *pNextPlugin);

protected:
	std::vector<PluginBase *> m_nextPluginVector;
};

class InputPluginBase;
class OutputPluginBase : public PluginBase{
public:
	OutputPluginBase();
	~OutputPluginBase();
	virtual void connect(InputPluginBase *pNextInputPlugin);

	virtual int start() = 0;// �Լ�ʵ�ֿ�ʼ��������output�������������һ��InputPluginBase

protected:
	void output(const void * data, int len);
};

class InputPluginBase : public PluginBase {
public:
	InputPluginBase(){}
	~InputPluginBase(){}
	virtual void connect(InputPluginBase *pNextOutputPlugin);

	virtual void input(const void * data, int len) = 0;// �Լ�ʵ�����ݴ���
};