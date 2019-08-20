#include "stdafx.h"
#include "RtmpSenderPlugin.h"


RtmpSenderPlugin::RtmpSenderPlugin() {
    m_rtmpSender.Connect("rtmp://101.132.187.172:1935/live1/xyz111222");
}

void RtmpSenderPlugin::Input(const void * data, int len) {
    x264_nal_t *nals = (x264_nal_t*)data;
    for (int i = 0; i < len; i++) {
        m_rtmpSender.SendH264Packet(&nals[i]);
    }
}

RtmpSenderPlugin::~RtmpSenderPlugin() {
}

InputPluginBase* GetRtmpSenderInstance() {
    InputPluginBase* pInstance = new  RtmpSenderPlugin();
    return pInstance;
}
