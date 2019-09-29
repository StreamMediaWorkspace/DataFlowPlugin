#include "stdafx.h"

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

bool RtmpSender::SendMetadataPacket(DataBuffer *pDataBuffer) {
    return 0;
    char metadata_buf[1024];
    char* pbuf = WriteMetadata(metadata_buf, pDataBuffer);
    return Send(metadata_buf, (int)(pbuf - metadata_buf), FLV_TAG_TYPE_META, pDataBuffer->TimeStamp());
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

char* RtmpSender::WriteMetadata(char* buf, DataBuffer *pDataBuffer) {
    char* pbuf = buf;
    //第一个包：
    //第1个字节表示AMF包类型，一般总是0x02，
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_STRING);
    //第2-3个字节为UI16类型值，标识字符串的长度，一般总是0x000A（“onMetaData”长度）。后面字节为具体的字符串，一般总为“onMetaData”（6F,6E,4D,65,74,61,44,61,74,61）
    pbuf = AmfStringToBytes(pbuf, "onMetaData");

    //第二个AMF包：
    //第1个字节表示AMF包类型，一般总是0x08，表示数组。
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_MIXEDARRAY);
    //第2-5个字节为UI32类型值，表示数组元素的个数。
    pbuf = UI32ToBytes(pbuf, 6);

    //后面即为各数组元素的封装，数组元素为元素名称和值组成的对。常见的数组元素如下表所示。
    /*duration:时长
    width:视频宽度
    height:视频高度
    videodatarate:视频码率
    framerate:视频帧率
    videocodecid:视频编码方式
    audiosamplerate:音频采样率
    audiosamplesize:音频采样精度
    stereo:是否为立体声
    audiocodecid:音频编码方式
    filesize:文件大小*/
    pbuf = AmfStringToBytes(pbuf, "width");
    pbuf = AmfDoubleToBytes(pbuf, pDataBuffer->Width());

    pbuf = AmfStringToBytes(pbuf, "height");
    pbuf = AmfDoubleToBytes(pbuf, pDataBuffer->Height());

    pbuf = AmfStringToBytes(pbuf, "videodatarate");
    pbuf = AmfDoubleToBytes(pbuf, 256000);

    pbuf = AmfStringToBytes(pbuf, "framerate");
    pbuf = AmfDoubleToBytes(pbuf, 20);

    pbuf = AmfStringToBytes(pbuf, "videocodecid");
    pbuf = AmfDoubleToBytes(pbuf, 0x07);
// 
//     if (m_metaAudioIn) {
//         pbuf = AmfStringToBytes(pbuf, "audiocodecid");
//         pbuf = AmfDoubleToBytes(pbuf, 0);
// 
//         pbuf = AmfStringToBytes(pbuf, "audiodatarate");
//         pbuf = AmfDoubleToBytes(pbuf, m_metaAudioIn.getAudioBitRate());
// 
//         pbuf = AmfStringToBytes(pbuf, "audiosamplerate");
//         pbuf = AmfDoubleToBytes(pbuf, m_metaAudioIn.getAudioSampleRate());
// 
//         pbuf = AmfStringToBytes(pbuf, "audiochannel");
//         pbuf = AmfDoubleToBytes(pbuf, m_metaAudioIn.getAudioChanel());
//     }
    // 0x00 0x00 0x09
    pbuf = AmfStringToBytes(pbuf, "");
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_OBJECT_END);

    return pbuf;
}

void RtmpSender::SendVideoDataPacket(DataBuffer* dataBuf, bool isKeyframe)
{
    int need_buf_size = dataBuf->BufLen() + 9;
    char* buf = new char[need_buf_size];
    char* pbuf = buf;

    unsigned char flag = 0;
    if (isKeyframe)
        flag = 0x17;
    else
        flag = 0x27;

    pbuf = UI08ToBytes(pbuf, flag);
    pbuf = UI08ToBytes(pbuf, 1);
    pbuf = UI08ToBytes(pbuf, 0);
    pbuf = UI08ToBytes(pbuf, 0);    // avc packet type (0, nalu)
    pbuf = UI08ToBytes(pbuf, 0);    // composition time
    pbuf = UI32ToBytes(pbuf, dataBuf->BufLen());    // composition time

    memcpy(pbuf, dataBuf->Buf(), dataBuf->BufLen());
    pbuf += dataBuf->BufLen();

    Send(buf, (int)(pbuf - buf), FLV_TAG_TYPE_VIDEO, dataBuf->TimeStamp());
    delete []buf;
}

int RtmpSender::SendH264Packet(x264_nal_t *nal) {return 0;
    if (!nal) {
        return -1;
    }

    if (nal->i_type == NAL_SPS) {
        m_sps.type = NAL_SPS;
        m_sps.data = (uint8_t *)malloc(nal->i_payload - 4);
        m_sps.size = nal->i_payload - 4;
        memcpy(m_sps.data, nal->p_payload + 4, nal->i_payload - 4);
        return 0;
    }
    else if (nal->i_type == NAL_PPS) {
        m_pps.type = NAL_PPS;
        m_pps.data = (uint8_t *)malloc(nal->i_payload - 4);
        m_pps.size = nal->i_payload - 4;
        memcpy(m_pps.data, nal->p_payload + 4, nal->i_payload - 4);
        return 0;
    }

    unsigned int size = nal->i_payload;
    unsigned char *nalData = nal->p_payload;

    if (nalData[2] == 0x00) {
        nalData += 4;
        size -= 4;
    } else if (nalData[2] == 0x01) {
        nalData += 3;
        size -= 3;
    }

    unsigned char *body = (unsigned char*)malloc(size + 9);
    memset(body, 0, size + 9);
    if (nal->i_type == NAL_SLICE_IDR) {
        body[0] = 0x17;
    } else {
        body[0] = 0x27;
    }

    body[1] = 0x01;
    body[2] = 0x00;
    body[3] = 0x00;
    body[4] = 0x00;
    body[5] = (size >> 24) & 0xff;
    body[6] = (size >> 16) & 0xff;
    body[7] = (size >> 8) & 0xff;
    body[8] = (size) & 0xff;

    if (nal->i_type == NAL_SLICE_IDR) {
        SendVideoSpsPps(m_pps.data, m_pps.size, m_sps.data, m_sps.size);
    }

    memcpy(&body[9], nalData, size);
    int bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, body, 9 + size, GetTimeStamp());
    free(body);
    return bRet;
}

/**
* ·¢ËÍÊÓÆµµÄspsºÍppsÐÅÏ¢
*
* @param pps ´æ´¢ÊÓÆµµÄppsÐÅÏ¢
* @param pps_len ÊÓÆµµÄppsÐÅÏ¢³¤¶È
* @param sps ´æ´¢ÊÓÆµµÄppsÐÅÏ¢
* @param sps_len ÊÓÆµµÄspsÐÅÏ¢³¤¶È
*
* @³É¹¦Ôò·µ»Ø 1 , Ê§°ÜÔò·µ»Ø0
*/
int RtmpSender::SendVideoSpsPps(unsigned char *pps, int pps_len, unsigned char * sps, int sps_len)
{return 0;
    RTMPPacket * packet = NULL;//rtmp°ü½á¹¹
    unsigned char * body = NULL;
    int i;
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + 1024);
    //RTMPPacket_Reset(packet);//ÖØÖÃpacket×´Ì¬
    memset(packet, 0, RTMP_HEAD_SIZE + 1024);
    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    body = (unsigned char *)packet->m_body;
    i = 0;
    body[i++] = 0x17;//keyframe AVC (h.264)
    body[i++] = 0x00;

    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    /*AVCDecoderConfigurationRecord*/
    body[i++] = 0x01;
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];
    body[i++] = 0xff;

    /*sps*/
    body[i++] = 0xe1;
    body[i++] = (sps_len >> 8) & 0xff;
    body[i++] = sps_len & 0xff;
    memcpy(&body[i], sps, sps_len);
    i += sps_len;

    /*pps*/
    body[i++] = 0x01;
    body[i++] = (pps_len >> 8) & 0xff;
    body[i++] = (pps_len) & 0xff;
    memcpy(&body[i], pps, pps_len);
    i += pps_len;

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = i;
    packet->m_nChannel = 0x04;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nInfoField2 = m_pRtmp->m_stream_id;

    /*µ÷ÓÃ·¢ËÍ½Ó¿Ú*/
    int nRet = RTMP_SendPacket(m_pRtmp, packet, TRUE);
    free(packet);    //ÊÍ·ÅÄÚ´æ
    return nRet;
}

int RtmpSender::SendAudioHeader(unsigned long nSampleRate, int nChannel) {return 0;

    RTMPPacket packet;
    RTMPPacket_Reset(&packet);
    RTMPPacket_Alloc(&packet, 4);

    packet.m_body[0] = 0xAF;// nSampleRate;  // MP3 AAC format 48000Hz
    packet.m_body[1] = 0x00;
    packet.m_body[2] = 0x11;
    packet.m_body[3] = 0x90;//0x10ÐÞ¸ÄÎª0x90,2016-1-19
    /*2. AudioTagHeader
ÒôÆµTagÍ·Ò»°ãÓÉÒ»¸ö×Ö½Ú¶¨Òå£¨AACÓÃÁ½¸ö×Ö½Ú£©£¬µÚÒ»¸ö×Ö½ÚµÄ¶¨ÒåÈçÏÂ£º
ÒôÆµ¸ñÊ½ 4bits | ²ÉÑùÂÊ 2bits | ²ÉÑù¾«¶È 1bits | ÉùµÀÊý 1bits|

ÒôÆµ¸ñÊ½ 4bits
0x00 = Linear PCM, platform endian
0x01 = ADPCM
0x02 = MP3
0x03 = Linear PCM, little endian
0x04 = Nellymoser 16-kHz mono
0x05 = Nellymoser 8-kHz mono
0x06 = Nellymoser
0x07 = G.711 A-law logarithmic PCM
0x08 = G.711 mu-law logarithmic PCM
0x09 = reserved
0x0A = AAC
0x0B = Speex
0x0E = MP3 8-Khz
0x0F = Device-specific sound

²ÉÑùÂÊ 2bits
0 = 5.5-kHz
1 = 11-kHz
2 = 22-kHz
3 = 44-kHz
¶ÔÓÚAAC×ÜÊÇ3£¬ÕâÀï¿´ÆðÀ´FLV²»Ö§³Ö48K AAC£¬ÆäÊµ²»ÊÇµÄ£¬ºóÃæ»¹ÊÇ¿ÉÒÔ¶¨ÒåÎª48K¡£

²ÉÑù¾«¶È 1bits
0 = snd8Bit
1 = snd16Bit
Ñ¹Ëõ¹ýµÄÒôÆµ¶¼ÊÇ16bit

ÉùµÀÊý 1bits
0 = sndMono
1 = sndStereo
¶ÔÓÚAAC×ÜÊÇ1


×ÛÉÏ£¬Èç¹ûÊÇAAC 48K 16±ÈÌØ¾«¶È Ë«ÉùµÀ±àÂë£¬¸Ã×Ö½ÚÎª 0b1010 1111 = 0xAF¡£
¿´µÚ2¸ö×Ö½Ú£¬Èç¹ûÒôÆµ¸ñÊ½AAC£¨0x0A£©£¬AudioTagHeaderÖÐ»á¶à³ö1¸ö×Ö½ÚµÄÊý¾ÝAACPacketType£¬Õâ¸ö×Ö¶ÎÀ´±íÊ¾AACAUDIODATAµÄÀàÐÍ£º
0x00 = AAC sequence header£¬ÀàËÆh.264µÄsps,pps£¬ÔÚFLVµÄÎÄ¼þÍ·²¿³öÏÖÒ»´Î¡£
0x01 = AAC raw£¬AACÊý¾Ý*/
    //packet.m_body[0] = 0b10101111;

    packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet.m_hasAbsTimestamp = 0;
    packet.m_nChannel = nChannel;
    packet.m_nTimeStamp = GetTimeStamp();
    packet.m_nInfoField2 = m_pRtmp->m_stream_id;
    packet.m_nBodySize = 4;

    //µ÷ÓÃ·¢ËÍ½Ó¿Ú
    int nRet = RTMP_SendPacket(m_pRtmp, &packet, TRUE);
    RTMPPacket_Free(&packet);//ÊÍ·ÅÄÚ´æ
    return nRet;
}

// int RtmpSender::SendAacSpec(unsigned char *spec_buf, int spec_len)
// {
//     RTMPPacket * packet;
//     unsigned char * body;
//     //ºóÃæµÄAACÊý¾Ý·¢ËÍµÄÊ±ºò£¬Ç°Ãæ7¸ö×Ö½Ú¶¼ÊÇÖ¡Í·Êý¾Ý£¬²»ÓÃ·¢ËÍ£¬°ÑºóÃæµÄÊý¾ÝÓÃRTMP·¢³öÈ¥¾ÍÐÐÁË¡£
//     spec_buf += 7;
//     spec_len -= 7;  /*spec data³¤¶È,Ò»°ãÊÇ2*/
// 
//     packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + spec_len + 2);
//     memset(packet, 0, RTMP_HEAD_SIZE);
// 
//     packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
//     body = (unsigned char *)packet->m_body;
// 
//     /*AF 00 + AAC RAW data*/
//     body[0] = 0xAF;
//     body[1] = 0x00;
//     memcpy(&body[2], spec_buf, spec_len);
// 
//     packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
//     packet->m_nBodySize = spec_len + 2;
//     packet->m_nChannel = 0x04;
//     packet->m_nTimeStamp = 0;
//     packet->m_hasAbsTimestamp = 0;
//     packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
//     packet->m_nInfoField2 = m_pRtmp->m_stream_id;
// 
//     /*µ÷ÓÃ·¢ËÍ½Ó¿Ú*/
//     RTMP_SendPacket(m_pRtmp, packet, TRUE);
// 
//     return TRUE;
//
int RtmpSender::SendAccPacket(unsigned long nSampleRate, int nChannel,
    unsigned char *data, int length) {return 0;
    long timeoffset = 0;

    static unsigned long countAudioTime = 0;
    if (countAudioTime > 0)
    {
        timeoffset = (countAudioTime - 1) *((int)(1024 * 1000 / 44100));//
    }
    countAudioTime++;

    int size = length + 2;
    RTMPPacket packet;
    RTMPPacket_Reset(&packet);
    RTMPPacket_Alloc(&packet, size);

    int i = 0;
    // MP3 AAC format 48000Hz
    packet.m_body[i++] = 0xAf;//nSampleRate;
    packet.m_body[i++] = 0x01;
    memcpy(&packet.m_body[i], data, length);

    packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet.m_hasAbsTimestamp = 0;
    packet.m_nChannel = nChannel;
    packet.m_nTimeStamp = GetTimeStamp();
    packet.m_nInfoField2 = m_pRtmp->m_stream_id;
    packet.m_nBodySize = size;

    int nRet = RTMP_SendPacket(m_pRtmp, &packet, TRUE);
    RTMPPacket_Free(&packet);
    return nRet;
}

int RtmpSender::Close() {
    if (m_pRtmp != NULL) {
        RTMP_Close(m_pRtmp);
        RTMP_Free(m_pRtmp);
        m_pRtmp = NULL;
    }
    return 0;
}

int RtmpSender::SendPacket(unsigned int nPacketType, unsigned char *data, unsigned int size, unsigned int nTimestamp)
{
    if (!RTMP_IsConnected(m_pRtmp)) {
        printf("[RtmpSender] SendPacket not connected\n");
        return -1;
    }
    nTimestamp = GetTimeStamp();
    RTMPPacket* packet;
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + size);
    memset(packet, 0, RTMP_HEAD_SIZE);
    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    packet->m_nBodySize = size;
    memcpy(packet->m_body, data, size);
    packet->m_hasAbsTimestamp = 0;
    packet->m_packetType = nPacketType;
    packet->m_nInfoField2 = m_pRtmp->m_stream_id;
    packet->m_nChannel = 0x04;

    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    if (RTMP_PACKET_TYPE_AUDIO == nPacketType && size != 4)
    {
        packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    }
    packet->m_nTimeStamp = nTimestamp;
    int nRet = 0;
    if (RTMP_IsConnected(m_pRtmp))
    {
        //nRet = RTMP_SendPacket(m_pRtmp, packet, TRUE);
    }
    free(packet);
    return nRet;
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

time_t RtmpSender::GetTimeStamp() {
    static time_t t0 = GetTickCount();// time(0);
    return GetTickCount() - t0;
    return time(0) - t0;
}

int RtmpSender::SendData(char *data, int size, int timeStamp, bool isMedium) {
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
    if (RTMP_IsConnected(m_pRtmp))
    {
        nRet = RTMP_SendPacket(m_pRtmp, packet, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
    }
    /*释放内存*/
    free(packet);
    return nRet;
}

