#include "Capture.h"
#include <list>
#include "../common/Logger.h"

Capture::Capture() :
    m_pVideoCapture(nullptr) {
}

Capture::~Capture() {
}

int Capture::Start() {
    m_pVideoCapture = std::make_shared<DShowCapture>(DShowCapture::euVideo, this);
    if (0 != m_pVideoCapture->Init()) {
        LogE("Capture Start Init failed\n");
        m_pVideoCapture.reset();
        return -1;
    }
    m_pVideoCapture->SetVideoFomat(m_width, m_height, m_fps);
    std::list<VideoFormat> frameRates;
    m_pVideoCapture->GetCameraSupportedResolution(frameRates);
    //m_pVideoCapture->SetCameraSupportedResolution(0);
    if (0 == m_pVideoCapture->Run()) {
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

void Capture::OnVideoData(DataBuffer *pDataBuffer) {
    LogI("OnVideoData data lenght=%d\n", pDataBuffer->BufLen());

//     FILE *f1 = NULL;
//     fopen_s(&f1, "./test2.yuv", "wb");
//     fwrite(pDataBuffer->Buf(), pDataBuffer->BufLen(), 1, f1);
//     fclose(f1);

    PluginBase::Input(pDataBuffer);
}

void Capture::OnAudioData(DataBuffer *pDataBuffer) {

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
        m_pVideoCapture->SetVideoFomat(m_width, m_height, m_fps);
        ret = 0;
    }
    PluginBase::Control(metaData);
    return ret;
}

PluginBase* GetCaptureInstance() {
    PluginBase* pInstance = new  Capture();
	return pInstance;
}

