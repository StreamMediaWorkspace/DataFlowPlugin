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
    OutputPluginBase::Output(s.c_str(), s.length());
}

void Capture::OnVideoData(void *data, int length) {
    LogI("OnVideoData %d\n", length);
}

void Capture::OnAudioData(void *data, int length) {

}

OutputPluginBase* GetCaptureInstance() {
	OutputPluginBase* pInstance = new  Capture();
	return pInstance;
}

