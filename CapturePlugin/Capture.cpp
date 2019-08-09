#include "Capture.h"

Capture::Capture() :
    m_pVideoCapture(nullptr) {
}

Capture::~Capture() {
}

int Capture::Start() {
    m_pVideoCapture = std::make_shared<DShowCapture>(DShowCapture::euVideo, this);
    return m_pVideoCapture->Run();

    LOG_FUNCTION;
    std::string s = "hello world";
    OutputPluginBase::Output(s.c_str(), s.length());
}

void Capture::OnVideoData(void *data, int length) {
    printf("OnVideoData %d\n", length);
}

void Capture::OnAudioData(void *data, int length) {

}

OutputPluginBase* GetCaptureInstance() {
	LOG_FUNCTION;
	OutputPluginBase* pInstance = new  Capture();
	return pInstance;
}

