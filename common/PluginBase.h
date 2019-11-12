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

class DataBuffer {
public:
    enum MediaType {euVideo, euAudio};
    DataBuffer(unsigned char* buf, 
        unsigned int bufLen,
        long long timeStamp,
        MediaType mediaType) {
        buf_ = buf;
        buf_len_ = bufLen;
        m_timeStamp = timeStamp;
        m_mediaType = mediaType;
    }

    DataBuffer(const DataBuffer& other) {
        buf_ = other.buf_;
        buf_len_ = other.buf_len_;
        m_timeStamp = other.m_timeStamp;
        m_mediaType = other.m_mediaType;
    }

    virtual unsigned char* Buf() { return buf_; }
    virtual unsigned int BufLen() { return buf_len_; }
    virtual long long TimeStamp() { return m_timeStamp; }
    virtual MediaType GetMediaType() { return m_mediaType;}

    ~DataBuffer() {
        free(buf_); buf_ = nullptr;
    }

private:
    unsigned char* buf_;
    unsigned int buf_len_;
    long long m_timeStamp;
    MediaType m_mediaType;
};

class VideoDataBuffer : public DataBuffer {
public:
    VideoDataBuffer(unsigned char* buf, unsigned int bufLen,
        unsigned int width, 
        unsigned int height,
        long long timeStamp)
        : DataBuffer(buf, bufLen, timeStamp, euVideo) {
        m_width = width;
        m_height = height;
    }

    VideoDataBuffer(const VideoDataBuffer& other) 
        : DataBuffer(other){
        m_width = other.m_width;
        m_height = other.m_height;
    }

    ~VideoDataBuffer() {
//         if (is_need_free_)
//         {
//             delete[] buf_;
//         }
    }

    unsigned int Width() { return m_width; }
    unsigned int Height() { return m_height; }

//     DataBuffer* Clone() {
//         unsigned char* buf_clone = new unsigned char[buf_len_];
//         memcpy(buf_clone, buf_, buf_len_);
//         DataBuffer* data_buf = new DataBuffer(buf_clone, buf_len_, m_width, m_height, m_timeStamp, m_tag_header, is_need_free_);
//         return data_buf;
//     }

private:
    int m_width;
    int m_height;
};

class AudioDataBuffer : public DataBuffer {
public:
    AudioDataBuffer(unsigned char* buf,
        unsigned int bufLen,
        unsigned int sameplesPerSec, 
        unsigned int bitsPerSameple, 
        unsigned int channels,
        long long timeStamp)
        : DataBuffer(buf, bufLen, timeStamp, euAudio) {
        m_sameplesPerSec = sameplesPerSec;
        m_bitsPersameple = bitsPerSameple;
        m_channels = channels;
    }

    unsigned int GetSameplesPerSec() { return m_sameplesPerSec; }
    unsigned int GetBitsPersameple() { return m_bitsPersameple; }
    unsigned int GetChannels() { return m_channels; }

    ~AudioDataBuffer() {
    }

private:
    unsigned int m_sameplesPerSec;
    unsigned int m_bitsPersameple;
    unsigned int m_channels;
};

class FaacAudioDataBuffer : public AudioDataBuffer {
public:
    enum FaacAudioType { euSpecificInfo, euBody };
    FaacAudioDataBuffer(unsigned char* buf,
        unsigned int bufLen,
        unsigned int sameplesPerSec,
        unsigned int bitsPerSameple,
        unsigned int channels,
        long long timeStamp, 
        FaacAudioType audioType)
        :AudioDataBuffer(buf,
            bufLen,
            sameplesPerSec,
            bitsPerSameple,
            channels,
            timeStamp) {
        m_audioType = audioType;
    }
    ~FaacAudioDataBuffer() {}
    FaacAudioType GetFaacAudioType() {
        return m_audioType;
    }
private:
    FaacAudioType m_audioType;
};

class X264DataBuffer : public VideoDataBuffer {
public:
    enum VideoType { euSPS, euPPS, euBody };
    X264DataBuffer(unsigned char* buf,
        unsigned int bufLen,
        unsigned int width,
        unsigned int height,
        long long timeStamp, VideoType type, bool isKeyFrame)
        : VideoDataBuffer(buf, bufLen, width, height, timeStamp) {
        m_type = type;
        m_isKeyFrame = isKeyFrame;
    }

    X264DataBuffer(const X264DataBuffer& other)
        : VideoDataBuffer(other) {
        m_type = other.m_type;
        m_isKeyFrame = other.m_isKeyFrame;
    }

    VideoType GetType() {
        return m_type;
    }

    bool IsKeyFrame() {
        return m_isKeyFrame;
    }
    
    ~X264DataBuffer() {
    
    }

private:
    VideoType m_type;
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