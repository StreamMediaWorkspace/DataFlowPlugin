#pragma once
#if _MSC_VER < 1900 //vs2015 already have this function
#define snprintf _snprintf_s 
#endif
#include "include/librtmp/rtmp_sys.h"
#include "include/librtmp/log.h"
#include "include/x264/x264.h"
#include "include/x264/x264_config.h"

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

    int SendH264Packet(x264_nal_t *nal);
    //int SendVideoSpsPps(void *data, int length);
    //int SendVideoPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp);
    int SendAccPacket(unsigned long nSampleRate, int nChannel, unsigned char *data, int length);
    int SendAudioHeader(unsigned long nSampleRate, int nChannel);
    int SendAacSpec(unsigned char *spec_buf, int spec_len);

private:
    int InitSockets();
    void CleanupSockets();
    time_t GetTimeStamp();

     int SendVideoSpsPps(unsigned char *pps, int pps_len, unsigned char * sps, int sps_len);
     
     int SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp);
    NaluUnit m_pps;
    NaluUnit m_sps;
private:
    RTMP *m_pRtmp;
};

