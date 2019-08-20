#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "X246Encode.h"
#include "../common/Logger.h"

#pragma comment(lib, "./lib/x264.lib")

X246Encode::X246Encode() {

}

void X246Encode::ParamsChanged(int width, int height, int fps) {
    m_width = width;
    m_height = height;
   
    x264_param_t m_param;
    x264_param_default_preset(&m_param, "veryfast", "zerolatency");
    m_param.i_threads = 1;
    m_param.i_width = m_width;
    m_param.i_height = m_height;
    m_param.i_fps_num = fps;
    m_param.i_bframe = 5;
    m_param.i_fps_den = 1;
    m_param.i_keyint_max = 3;
    m_param.b_intra_refresh = 1;
    m_param.b_annexb = 1;
    m_param.rc.f_rf_constant = 1;
    x264_param_apply_profile(&m_param, "high422");
    if (m_encoder) {
        x264_encoder_close(m_encoder);
    }
    m_encoder = x264_encoder_open(&m_param);
    m_picIn.i_pts = 0;

    //x264_encoder_parameters(m_encoder, &m_param);
}

int X246Encode::Encode(uint8_t* yuvBuffer, x264_nal_t **pp_nal, int *pi_nal) {
    if (!m_encoder) {
        LogE("X246Encode::Encode m_encoder is null\n");
        return -1;
    }
    if (0 != x264_picture_alloc(&m_picIn, X264_CSP_I420, m_width, m_height)) {
        printf("x264_picture_alloc failed\n");
        return -1;
    }

    m_picIn.img.plane[0] = yuvBuffer;
    m_picIn.img.plane[1] = m_picIn.img.plane[0] + m_width * m_height;
    m_picIn.img.plane[2] = m_picIn.img.plane[1] + m_width * m_height / 4;
    m_picIn.i_pts++;

    return x264_encoder_encode(m_encoder, pp_nal, pi_nal, &m_picIn, &m_picOut);
}

X246Encode::~X246Encode() {
    if (m_encoder) {
        x264_encoder_close(m_encoder);
    }
}
