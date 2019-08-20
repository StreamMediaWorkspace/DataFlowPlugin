#pragma once
#include <stdint.h>
#include "include/x264/x264.h"
#include "include/x264/x264_config.h"

class X246Encode
{
public:
    X246Encode();
    
    ~X246Encode();
    void ParamsChanged(int width, int height, int fps);
    int Encode(uint8_t* yuvBuffer, x264_nal_t **pp_nal, int *pi_nal);

private:
    x264_t *m_encoder = NULL;
    x264_picture_t m_picIn;
    x264_picture_t m_picOut;

    int m_width;
    int m_height;
};

