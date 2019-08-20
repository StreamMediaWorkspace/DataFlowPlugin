#pragma once
#include <vector>
#include <assert.h>
#include <map>
#define _LIB

class MetaData {
public:
    MetaData() {}
    ~MetaData() {}

    int setProperty(const char *key, int value) {
        if (key == NULL)
            return -2;
        m_intProperties[key] = value;
        return 0;
    }

    int getProperty(const char *key, int& value) {
        if (key == NULL)
            return -2;
        std::map<std::string, int>::iterator itr = m_intProperties.find(key);
        if (itr == m_intProperties.end()) {
            return -1;
        }
        value = itr->second;
        return 0;
    }

    int setProperty(const char *key, const std::string& value) {
        if (key == NULL)
            return -2;
        m_strProperties[key] = value;
        return 0;
    }

    int getProperty(const char *key, std::string& value) {
        if (key == NULL)
            return -2;
        std::map<std::string, std::string>::iterator itr = m_strProperties.find(key);
        if (itr == m_strProperties.end()) {
            return -1;
        }
        value = itr->second;
        return 0;
    }

private:
    std::map<std::string, std::string> m_strProperties;
    std::map<std::string, int> m_intProperties;
};

class PluginBase
{
public:
	PluginBase();
	~PluginBase();
	
	virtual const char* GetName() = 0;
	virtual const char* GetVersion() = 0;

    virtual int Control(MetaData madaData);

protected:
	virtual void Connect(PluginBase *pNextPlugin);

protected:
	std::vector<PluginBase *> m_nextPluginVector;
};

class InputPluginBase;
class OutputPluginBase : public PluginBase{
public:
	OutputPluginBase();
	~OutputPluginBase();
	virtual void Connect(InputPluginBase *pNextInputPlugin);

	virtual int Start() = 0;// 自己实现开始，并调用output将数据输出给下一个InputPluginBase

protected:
	void Output(const void * data, int len);
};

class InputPluginBase : public PluginBase {
public:
	InputPluginBase(){}
	~InputPluginBase(){}
	virtual void Connect(InputPluginBase *pNextOutputPlugin);

	virtual void Input(const void * data, int len) = 0;// 自己实现数据处理
};

class TransformPluginBase : public InputPluginBase {
public:
    TransformPluginBase() {}
    ~TransformPluginBase() {}

protected:
    virtual void Output(const void * data, int len);
};