#include "stdafx.h"
#include "X264Encoder.h"
#include "../common/Logger.h"

#include "../common/AmfByteStream.h"
#pragma comment(lib, "./lib/x264.lib")

X264Encoder::X264Encoder()
{
    m_encoder = NULL;
}

X264Encoder::~X264Encoder() {
    Destroy();
}

int X264Encoder::Initialize(int iWidth, int iHeight, int iRateBit, int iFps) {
    //     m_width = iWidth;
    //     m_height = iHeight;

    x264_param_t m_param;
    x264_param_default_preset(&m_param, "veryfast", "zerolatency");
    m_param.i_threads = 1;
    m_param.i_width = iWidth;
    m_param.i_height = iHeight;
    m_param.i_fps_num = iFps;
    m_param.i_bframe = 5;
    m_param.i_fps_den = 1;
    m_param.i_keyint_max = 3;
    m_param.b_intra_refresh = 1;
    m_param.b_annexb = 1;
    m_param.rc.f_rf_constant = 1;
    x264_param_apply_profile(&m_param, "high422");
    m_encoder = x264_encoder_open(&m_param);
    m_picIn.i_pts = 0;
    return 0;
}

int X264Encoder::Destroy() {
    if (m_encoder) {
        x264_picture_clean(&m_picIn);
        x264_picture_clean(&m_picOut);
        x264_encoder_close(m_encoder);
    }

    return 0;
}

int X264Encoder::Encode(unsigned char* szYUVFrame, int width, int height,
    x264_nal_t *&nals, int& nalsCount, bool& isKeyframe)
{
    if (0 != x264_picture_alloc(&m_picIn, X264_CSP_I420, width, height)) {
        LogE("x264_picture_alloc failed\n");
        return -1;
    }
    m_picIn.img.plane[0] = szYUVFrame;
    m_picIn.img.plane[1] = m_picIn.img.plane[0] + width * height;
    m_picIn.img.plane[2] = m_picIn.img.plane[1] + width * height / 4;
    m_picIn.i_pts++;

    if (isKeyframe) {
        m_picIn.i_type = X264_TYPE_KEYFRAME;
    }
    int ret = x264_encoder_encode(m_encoder, &nals, &nalsCount, &m_picIn, &m_picOut);
    if (ret <= 0) {
        LogE("x264 [error]: x264_encoder_encode failed ret %d\n", ret);
        return -1;
    }
    return 0;
}