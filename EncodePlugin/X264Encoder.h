#ifndef _X264_ENCODER_H_
#define _X264_ENCODER_H_

#include "stdint.h"
//#include <cstdio>

extern "C"
{
#include "./include/x264/x264.h"
}

class X264Encoder
{
public:
    X264Encoder();
    ~X264Encoder();

    void UpdateSps(const x264_nal_t *nal);
    void UpdatePps(const x264_nal_t *nal);
    int Encode(unsigned char* szYUVFrame, int width, int height,
        x264_nal_t *&nals, int& nalsCount, bool& isKeyframe);
    
    // ³õÊ¼»¯±àÂëÆ÷
    int Initialize(int iWidth, int iHeight, int iRateBit = 96, int iFps = 25);

    unsigned char* Add264SequenceHeader(unsigned char* outBuf, int& outLen);
    int GetAVCSequenceHeader(unsigned char** outBuf, int& outLen);

    // Ïú»Ù±àÂëÆ÷
    int Destroy();

private:
    x264_t *m_encoder;
    x264_picture_t m_picIn;
    x264_picture_t m_picOut;
};

#endif // _X264_ENCODER_H_
