#pragma once
#include <vector>
#include <assert.h>
#include <map>
#include <deque>
#include <mutex>
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

class DataBuffer
{
    enum DataType {euVideoGBK24, euVideoYUY2, euNone};
public:
    DataBuffer(unsigned char* buf, unsigned int bufLen, 
        unsigned int width, 
        unsigned int height,
        long long timeStamp, 
        bool tag_header, 
        bool isNeedFree = false)
    {
        buf_ = buf;
        buf_len_ = bufLen;
        m_width = width;
        m_height = height;
        is_need_free_ = isNeedFree;
        m_timeStamp = timeStamp;
        m_tag_header = tag_header;
    }

    DataBuffer(const DataBuffer& other)
    {
        buf_ = other.buf_;
        buf_len_ = other.buf_len_;
        is_need_free_ = other.is_need_free_;
        m_width = other.m_width;
        m_height = other.m_height;
        m_timeStamp = other.m_timeStamp;
        m_tag_header = other.m_tag_header;
    }

    ~DataBuffer()
    {
//         if (is_need_free_)
//         {
//             delete[] buf_;
//         }
    }

    virtual unsigned char* Buf() { return buf_; }

    virtual unsigned int BufLen() { return buf_len_; }
    virtual unsigned int Width() { return m_width; }
    virtual unsigned int Height() { return m_height; }
    virtual long long TimeStamp() { return m_timeStamp; }
    virtual bool TagHeader() {
        return m_tag_header;
    }

//     DataBuffer* Clone() {
//         unsigned char* buf_clone = new unsigned char[buf_len_];
//         memcpy(buf_clone, buf_, buf_len_);
//         DataBuffer* data_buf = new DataBuffer(buf_clone, buf_len_, m_width, m_height, m_timeStamp, m_tag_header, is_need_free_);
//         return data_buf;
//     }

private:
    unsigned char* buf_;
    unsigned int buf_len_;
    bool is_need_free_;
    int m_width;
    int m_height;
    long long m_timeStamp;

    bool m_tag_header = false;
};

class X264DataBuffer : public DataBuffer {
public:
    enum Type { euSPS, euPPS, euBody };
    X264DataBuffer(unsigned char* buf,
        unsigned int bufLen,
        unsigned int width,
        unsigned int height,
        long long timeStamp, Type type, bool isKeyFrame)
        : DataBuffer(buf, bufLen, width, height, timeStamp, false) {
        m_type = type;
        m_isKeyFrame = isKeyFrame;
    }

    Type GetType() {
        return m_type;
    }

    bool IsKeyFrame() {
        return m_isKeyFrame;
    }
    
    ~X264DataBuffer() {
    
    }

private:
    Type m_type;
    bool m_isKeyFrame = false;
};

class DataBufferQueue {
public:
    DataBufferQueue() {}
    ~DataBufferQueue() {}
    void push(DataBuffer *pDataBuffer) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_dataBufferQueue.push_back(pDataBuffer);
    }

    DataBuffer *pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_dataBufferQueue.size() == 0) {
            return NULL;
        }

        DataBuffer *pDataBuffer = m_dataBufferQueue.front();
        m_dataBufferQueue.pop_front();
        return pDataBuffer;
    }

private:
    std::deque<DataBuffer*> m_dataBufferQueue;
    std::mutex m_mutex;
};

class PluginBase
{
public:
	PluginBase();
	~PluginBase();
	
	virtual const char* GetName() = 0;
	virtual const char* GetVersion() = 0;

    virtual int Control(MetaData madaData);//在开始后或开始前调用，必须connect后调用才能传给下一个
    virtual int Start();//开始流程
    virtual int Stop();//停止流程

	virtual void Connect(PluginBase *pNextPlugin);

protected:
    virtual void Input(DataBuffer *pdataBuffer);

protected:
	std::vector<PluginBase *> m_nextPluginVector;
};

/*class InputPluginBase;
class OutputPluginBase : public PluginBase{
public:
	OutputPluginBase();
	~OutputPluginBase();
	virtual void Connect(InputPluginBase *pNextInputPlugin);

protected:
	void Output(DataBuffer *pDataBuffer);
};

class InputPluginBase : public PluginBase {
public:
	InputPluginBase(){}
	~InputPluginBase(){}
	virtual void Connect(InputPluginBase *pNextOutputPlugin);

	virtual void Input(DataBuffer *pDataBuffer) = 0;// 自己实现数据处理
};

class TransformPluginBase : public InputPluginBase {
public:
    TransformPluginBase() {}
    ~TransformPluginBase() {}

protected:
    virtual void Output(DataBuffer *pDataBuffer);
};*/