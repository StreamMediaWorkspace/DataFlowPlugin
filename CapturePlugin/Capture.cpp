#include "Capture.h"
#include <list>
#include "../common/Logger.h"

Capture::Capture() :
    m_pVideoCapture(nullptr) {
}

Capture::~Capture() {
}

int Capture::Start() {
    std::map<std::string, std::string> videoDeviceList;
    DShowCaptureDevice::ListCapDevices(CLSID_VideoInputDeviceCategory, videoDeviceList);

    m_pVideoCapture = std::make_shared<DShowVideoCapture>(this);
    m_pVideoCapture->SetDevice("@device:pnp:\\\\?\\usb#vid_04f2&pid_b5ab&mi_00#6&b0e92e&0&0000#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\\global");
    m_pVideoCapture->SetVideoFormat(m_width, m_height, m_fps);

    if (0 != m_pVideoCapture->Init()) {
        LogE("Capture Start Init failed\n");
        m_pVideoCapture.reset();
        return -1;
    }
    
    std::list<VideoFormat> frameRates;
    m_pVideoCapture->GetCameraSupportedResolution(frameRates);
    //m_pVideoCapture->SetCameraSupportedResolution(0);

    std::map<std::string, std::string> audioDeviceList;
    DShowCaptureDevice::ListCapDevices(CLSID_AudioInputDeviceCategory, audioDeviceList);
    m_pAudioCapture = std::make_shared<DShowAudioCapture>(this);
    m_pAudioCapture->SetDevice("@device:cm:{33D9A762-90C8-11D0-BD43-00A0C911CE86}\\wave:{AC65F880-46B6-472E-B63B-920D45B501F2}");
    //m_pAudioCapture->SetAudioFormat(m_width, m_height, m_fps);

    if (0 != m_pAudioCapture->Init()) {
        LogE("audio Capture Start Init failed\n");
        m_pAudioCapture.reset();
        return -1;
    }

    if (0 == m_pVideoCapture->Start() && 0 == m_pAudioCapture->Start()) {
        return PluginBase::Start();
    }
    return -1;
}

int Capture::Stop() {
    int ret = -1;
    if (m_pVideoCapture) {
        ret = m_pVideoCapture->Stop();
    }
    else {
        LogE("Capture Stop failed, m_pVideoCapture is null\n");
    }
    if (0 == ret) {
        ret = PluginBase::Start();
    }
    return ret;
}

void Capture::OnData(DataBuffer *pDataBuffer) {
    LogI("OnVideoData data lenght=%d\n", pDataBuffer->BufLen());

//     FILE *f1 = NULL;
//     fopen_s(&f1, "./test2.yuv", "wb");
//     fwrite(pDataBuffer->Buf(), pDataBuffer->BufLen(), 1, f1);
//     fclose(f1);

    PluginBase::Input(pDataBuffer);
}

int Capture::Control(MetaData metaData) {
    int ret = -1;
    int value = -1;
    if (metaData.getProperty(CAPTURE_PLUGIN_KEY_WIDTH, value) == 0) {
        m_width = (int)value;
    }
    
    if (metaData.getProperty(CAPTURE_PLUGIN_KEY_HEIGHT, value) == 0) {
        m_height = (int)value;
    }
    
    if (metaData.getProperty(CAPTURE_PLUGIN_KEY_FPS, value) == 0) {
        m_fps = (int)value;
    }

    if (m_width > 0 && m_height > 0 && m_fps > 0 && m_pVideoCapture) {
        m_pVideoCapture->SetVideoFormat(m_width, m_height, m_fps);
        ret = 0;
    }
    PluginBase::Control(metaData);
    return ret;
}

PluginBase* GetCaptureInstance() {
    PluginBase* pInstance = new  Capture();
	return pInstance;
}

