#pragma once
#if _MSC_VER < 1900 //vs2015 already have this function
#define snprintf _snprintf_s 
#endif
#include "include/librtmp/rtmp_sys.h"
#include "include/librtmp/log.h"
#include "include/x264/x264.h"
#include "include/x264/x264_config.h"

#include "../common/PluginBase.h"

//定义包头长度，RTMP_MAX_HEADER_SIZE=18
#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)

typedef struct NaluUnit
{
    int type;
    int size;
    unsigned char *data;
}NaluUnit;

class RtmpSender
{
public:
    RtmpSender();
    ~RtmpSender();

    int Connect(const char *url);
    int Close();

    int SendVideoData(char *data, int length, int timestamp, bool isMedium);

    int SendAudioData(char *data, int length, int timestamp, bool isMedium);

    bool Send(const char* buf, int bufLen, int type, unsigned int timestamp);
    
private:
    int InitSockets();
    void CleanupSockets();

private:
    RTMP *m_pRtmp;
};

