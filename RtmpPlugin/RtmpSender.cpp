#include "stdafx.h"
#include "../common/Logger.h"
#include "RtmpSender.h"
#include "../common/AmfByteStream.h"
#include <time.h>
#include <stdlib.h>

#pragma comment(lib, "./lib/librtmp.lib")
#pragma comment(lib,"WS2_32.lib") 

RtmpSender::RtmpSender()
{
    InitSockets();
}

int RtmpSender::Connect(const char* url) {

    m_pRtmp = RTMP_Alloc();
    RTMP_Init(m_pRtmp);


    m_pRtmp->Link.timeout = 5;
    if (!RTMP_SetupURL(m_pRtmp, (char*)url))
    {
        RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
        RTMP_Free(m_pRtmp);
        CleanupSockets();
        return -1;
    }

    //if unable,the AMF command would be 'play' instead of 'publish'
    RTMP_EnableWrite(m_pRtmp);
    RTMP_SetBufferMS(m_pRtmp, 10000 * 1000);
    if (!RTMP_Connect(m_pRtmp, NULL)) {
        RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
        RTMP_Free(m_pRtmp);
        m_pRtmp = NULL;
        CleanupSockets();
        return -1;
    }

    if (!RTMP_ConnectStream(m_pRtmp, 0)) {
        RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
        RTMP_Close(m_pRtmp);
        RTMP_Free(m_pRtmp);
        m_pRtmp = NULL;
        CleanupSockets();
        return -1;
    }

    return 0;
}

bool RtmpSender::Send(const char* buf, int bufLen, int type, unsigned int timestamp) {
    int nRtmpResult = RTMP_IsConnected(m_pRtmp);
    if (nRtmpResult == 0) {
        return false;
    }
    RTMPPacket rtmp_pakt;
    RTMPPacket_Reset(&rtmp_pakt);
    RTMPPacket_Alloc(&rtmp_pakt, bufLen);

    rtmp_pakt.m_packetType = type;
    rtmp_pakt.m_nBodySize = bufLen;
    rtmp_pakt.m_nTimeStamp = timestamp;
    rtmp_pakt.m_nChannel = 4;
    rtmp_pakt.m_headerType = RTMP_PACKET_SIZE_LARGE;
    rtmp_pakt.m_nInfoField2 = m_pRtmp->m_stream_id;
    memcpy(rtmp_pakt.m_body, buf, bufLen);

    int retval = RTMP_SendPacket(m_pRtmp, &rtmp_pakt, 0);
    RTMPPacket_Free(&rtmp_pakt);

    return !!retval;
}

int RtmpSender::Close() {
    if (m_pRtmp != NULL) {
        RTMP_Close(m_pRtmp);
        RTMP_Free(m_pRtmp);
        m_pRtmp = NULL;
    }
    return 0;
}

int RtmpSender::InitSockets() {
#ifdef WIN32
    WORD version;
    WSADATA wsaData;
    version = MAKEWORD(2, 2);
    return (WSAStartup(version, &wsaData) == 0);
#endif
}

void RtmpSender::CleanupSockets()
{
#ifdef WIN32
    WSACleanup();
#endif
}

RtmpSender::~RtmpSender() {
    CleanupSockets();
}

int RtmpSender::SendVideoData(char *data, int size, int timeStamp, bool isMedium) {
    LogI("RtmpSender SendVideoData size=%d, timeStamp=%d, isMedium=%d\n", size, timeStamp, isMedium);
    if (!RTMP_IsConnected(m_pRtmp)) {
        printf("[RtmpSender] SendPacket not connected\n");
        return -1;
    }
    RTMPPacket* packet;
    /*分配包内存和初始化,len为包体长度*/
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + size);
    memset(packet, 0, RTMP_HEAD_SIZE);
    /*包体内存*/
    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    packet->m_nBodySize = size;
    memcpy(packet->m_body, data, size);
    packet->m_hasAbsTimestamp = 0;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    if (isMedium) {
        packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM; /*此处为类型有两种一种是音频,一种是视频*/
    } else {
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    }
    packet->m_nInfoField2 = m_pRtmp->m_stream_id;
    packet->m_nChannel = 0x04;

    //packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    //     if (RTMP_PACKET_TYPE_AUDIO == nPacketType && size != 4)
    //     {
    //         packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    //     }
    packet->m_nTimeStamp = timeStamp;
    /*发送*/
    int nRet = 0;
    if (RTMP_IsConnected(m_pRtmp)) {
        nRet = RTMP_SendPacket(m_pRtmp, packet, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
    }
    /*释放内存*/
    free(packet);
    return nRet;
}

int RtmpSender::SendAudioData(char *data, int size, int timestamp, bool isMedium) {
    if (!RTMP_IsConnected(m_pRtmp)) {
        printf("[RtmpSender] SendPacket not connected\n");
        return -1;
    }
    RTMPPacket* packet;
    /*分配包内存和初始化,len为包体长度*/
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + size);
    memset(packet, 0, RTMP_HEAD_SIZE);
    /*包体内存*/
    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    packet->m_nBodySize = size;
    memcpy(packet->m_body, data, size);
    packet->m_hasAbsTimestamp = 0;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    if (!isMedium) {
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE; /*此处为类型有两种一种是音频,一种是视频*/
    } else {
        packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    }
    packet->m_nInfoField2 = m_pRtmp->m_stream_id;
    packet->m_nChannel = 0x05;
    packet->m_nTimeStamp = timestamp;
    /*发送*/
    int nRet = 0;
    if (RTMP_IsConnected(m_pRtmp)) {
        nRet = RTMP_SendPacket(m_pRtmp, packet, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
    }
    /*释放内存*/
    free(packet);
    return nRet;
}

