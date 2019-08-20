// EncodePlugin.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "EncodePlugin.h"
#include "../common/Logger.h"

EncodePlugin::EncodePlugin() {

}

EncodePlugin::~EncodePlugin() {
}

int EncodePlugin::Control(MetaData metaData) {
    int ret = -1;
    int value = -1;
    if (metaData.getProperty(ENCODE_PLUGIN_KEY_WIDTH, value) == 0) {
        m_nWidth = (int)value;
    }
    
    if (metaData.getProperty(ENCODE_PLUGIN_KEY_HEIGHT, value) == 0) {
        m_nHeight = (int)value;
    }
    
    if (metaData.getProperty(ENCODE_PLUGIN_KEY_FPS, value) == 0) {
        m_nFps = (int)value;
    }

    if (m_nWidth > 0 && m_nHeight > 0 && m_nFps > 0) {
        m_X246Encode.ParamsChanged(m_nWidth, m_nHeight, m_nFps);
        ret = 0;
        LogI("EncodePlugin::Control width=%d,height=%d,fps=%d\n", m_nWidth, m_nHeight, m_nFps);
    }
    TransformPluginBase::Control(metaData);
    return ret;
}

void EncodePlugin::Input(const void * data, int len) {
    LogI("EncodePlugin::Input data=%p\n", data);

    x264_nal_t *nals;
    int nnal;
    m_X246Encode.Encode((uint8_t*)data, &nals, &nnal);
    TransformPluginBase::Output((void*)nals, nnal);
//     for (int i = 0; i < nnal; i++) {
//         LogI("-----nals[i].i_type=%d\n", nals[i].i_type);
//         //rtmpSender.SendH264Packet(&nals[i]);
//         
//         //Sleep(1000 / fps);
//     }
}

TransformPluginBase* GetEncodeInstance() {
    TransformPluginBase* pInstance = new  EncodePlugin();
    return pInstance;
}
