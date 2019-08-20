#include "Capture.h"
#include "../common/Logger.h"

Capture::Capture() :
    m_pVideoCapture(nullptr) {
}

Capture::~Capture() {
}

int Capture::Start() {
    m_pVideoCapture = std::make_shared<DShowCapture>(DShowCapture::euVideo, this);
    return m_pVideoCapture->Run();

    std::string s = "hello world";
}

void Capture::OnVideoData(void *data, int length) {
    LogI("OnVideoData data=%p\n", data);
    OutputPluginBase::Output(data, length);
}

void Capture::OnAudioData(void *data, int length) {

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
    OutputPluginBase::Control(metaData);
    return ret;
}

OutputPluginBase* GetCaptureInstance() {
	OutputPluginBase* pInstance = new  Capture();
	return pInstance;
}

