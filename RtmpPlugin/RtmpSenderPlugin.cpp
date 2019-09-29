#include "stdafx.h"
#include "../common/AmfByteStream.h"
#include "RtmpSenderPlugin.h"


RtmpSenderPlugin::RtmpSenderPlugin() {
}

int RtmpSenderPlugin::Start() {
    if (m_pThread) {
        LogE("RtmpSenderPlugin::Start failed, m_pThread is not null\n");
        return -1;
    }
    m_stop = false;
    m_rtmpSender.Connect("rtmp://101.132.187.172/live1/xyz111222");
    m_pThread = new std::thread(SenderThread, this);
    return PluginBase::Start();
}

int RtmpSenderPlugin::Stop() {
    m_stop = true;
    if (m_pThread &&m_pThread->joinable()) {
        m_pThread->join();
    }

    return PluginBase::Stop();
}

void RtmpSenderPlugin::Input(DataBuffer *pDataBuffer) {
    m_dataBufferQueue.push(pDataBuffer);
    PluginBase::Input(pDataBuffer);
}

void RtmpSenderPlugin::SenderThread(RtmpSenderPlugin* pRtmpSenderPlugin) {
    if (!pRtmpSenderPlugin) {
        LogE("RtmpSenderPlugin SenderThread pRtmpSenderPlugin is null\n");
        return;
    }
    pRtmpSenderPlugin->Loop();
}

void RtmpSenderPlugin::Loop() {
    X264DataBuffer *pDataBuffer = NULL;
    int index = 0;
    while (!m_stop) {
        {
            pDataBuffer = dynamic_cast<X264DataBuffer*>(m_dataBufferQueue.pop());
        }
        if (pDataBuffer) {
            if (pDataBuffer->GetType() == X264DataBuffer::euPPS) {
                UpdatePPS((char*)pDataBuffer->Buf(), pDataBuffer->BufLen());
            }
            else if (pDataBuffer->GetType() == X264DataBuffer::euSPS) {
                UpdateSPS((char*)pDataBuffer->Buf(), pDataBuffer->BufLen());
            }
            else {
                if (pDataBuffer->IsKeyFrame()) {
                    char *buffer = NULL;
                    int len = MetaDataToBuffer(pDataBuffer->Width(), pDataBuffer->Height(), &buffer);
                    int ret = m_rtmpSender.SendData(buffer, len, pDataBuffer->TimeStamp(), true);
                    free(buffer);

                    len = SpsPpsToBuffer(&buffer);
                    ret = m_rtmpSender.SendData(buffer, len, pDataBuffer->TimeStamp(), true);
                    free(buffer);
                }

                char *buffer = NULL;
                int len = BodyToBuffer((char*)pDataBuffer->Buf(), pDataBuffer->BufLen(),
                    pDataBuffer->IsKeyFrame(), &buffer);
                int ret = m_rtmpSender.SendData(buffer, len, pDataBuffer->TimeStamp(), false);
                free(buffer);
            }

            index++;
            delete pDataBuffer;
            pDataBuffer = NULL;
        }
    }
}

void RtmpSenderPlugin::UpdatePPS(char *pps, int ppsSize) {
    if (m_pps) {
        free(m_pps);
    }

    m_ppsSize = ppsSize;
    m_pps = (char*)malloc(m_ppsSize);
    memcpy(m_pps, pps, m_ppsSize);
}

void RtmpSenderPlugin::UpdateSPS(char *sps, int spsSize) {
    if (m_sps) {
        free(m_sps);
    }

    m_spsSize = spsSize;
    m_sps = (char*)malloc(m_spsSize);
    memcpy(m_sps, sps, m_spsSize);
}

int RtmpSenderPlugin::MetaDataToBuffer(int width, int height, char **outBuffer) {
    int bufferSize = 256;

    char* pbuf = *outBuffer = (char*)malloc(bufferSize);
    //��һ������
    //��1���ֽڱ�ʾAMF�����ͣ�һ������0x02��
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_STRING);
    //��2-3���ֽ�ΪUI16����ֵ����ʶ�ַ����ĳ��ȣ�һ������0x000A����onMetaData�����ȣ��������ֽ�Ϊ������ַ�����һ����Ϊ��onMetaData����6F,6E,4D,65,74,61,44,61,74,61��
    pbuf = AmfStringToBytes(pbuf, "onMetaData");

    //�ڶ���AMF����
    //��1���ֽڱ�ʾAMF�����ͣ�һ������0x08����ʾ���顣
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_MIXEDARRAY);
    //��2-5���ֽ�ΪUI32����ֵ����ʾ����Ԫ�صĸ�����
    pbuf = UI32ToBytes(pbuf, 6);

    //���漴Ϊ������Ԫ�صķ�װ������Ԫ��ΪԪ�����ƺ�ֵ��ɵĶԡ�����������Ԫ�����±���ʾ��
    /*duration:ʱ��
    width:��Ƶ���
    height:��Ƶ�߶�
    videodatarate:��Ƶ����
    framerate:��Ƶ֡��
    videocodecid:��Ƶ���뷽ʽ
    audiosamplerate:��Ƶ������
    audiosamplesize:��Ƶ��������
    stereo:�Ƿ�Ϊ������
    audiocodecid:��Ƶ���뷽ʽ
    filesize:�ļ���С*/
    pbuf = AmfStringToBytes(pbuf, "width");
    pbuf = AmfDoubleToBytes(pbuf, width);

    pbuf = AmfStringToBytes(pbuf, "height");
    pbuf = AmfDoubleToBytes(pbuf, height);

    pbuf = AmfStringToBytes(pbuf, "videodatarate");
    pbuf = AmfDoubleToBytes(pbuf, 256000);

    pbuf = AmfStringToBytes(pbuf, "framerate");
    pbuf = AmfDoubleToBytes(pbuf, 20);

    pbuf = AmfStringToBytes(pbuf, "videocodecid");
    pbuf = AmfDoubleToBytes(pbuf, 0x07);
    // 
    //     if (m_metaAudioIn) {
    //         pbuf = AmfStringToBytes(pbuf, "audiocodecid");
    //         pbuf = AmfDoubleToBytes(pbuf, 0);
    // 
    //         pbuf = AmfStringToBytes(pbuf, "audiodatarate");
    //         pbuf = AmfDoubleToBytes(pbuf, m_metaAudioIn.getAudioBitRate());
    // 
    //         pbuf = AmfStringToBytes(pbuf, "audiosamplerate");
    //         pbuf = AmfDoubleToBytes(pbuf, m_metaAudioIn.getAudioSampleRate());
    // 
    //         pbuf = AmfStringToBytes(pbuf, "audiochannel");
    //         pbuf = AmfDoubleToBytes(pbuf, m_metaAudioIn.getAudioChanel());
    //     }
    // 0x00 0x00 0x09
    pbuf = AmfStringToBytes(pbuf, "");
    pbuf = UI08ToBytes(pbuf, AMF_DATA_TYPE_OBJECT_END);

    return (pbuf - *outBuffer);
}

int RtmpSenderPlugin::SpsPpsToBuffer(char **outBuffer) {
    if (!m_pps || !m_sps) {
        LogE("SpsPpsToBuffer error\n");
        return 0;
    }
    int bufferSize = m_ppsSize + m_spsSize + 16;
    char *buffer = *outBuffer = (char*)malloc(bufferSize);
    int i = 0;
    buffer[i++] = 0x17;//keyframe AVC (h.264)
    buffer[i++] = 0x00;

    buffer[i++] = 0x00;
    buffer[i++] = 0x00;
    buffer[i++] = 0x00;

    /*AVCDecoderConfigurationRecord*/
    buffer[i++] = 0x01;
    buffer[i++] = m_sps[1];
    buffer[i++] = m_sps[2];
    buffer[i++] = m_sps[3];
    buffer[i++] = 0xff;

    /*sps*/
    buffer[i++] = 0xe1;
    buffer[i++] = (m_spsSize >> 8) & 0xff;
    buffer[i++] = m_spsSize & 0xff;
    memcpy(&buffer[i], m_sps, m_spsSize);
    i += m_spsSize;

    /*pps*/
    buffer[i++] = 0x01;
    buffer[i++] = (m_ppsSize >> 8) & 0xff;
    buffer[i++] = (m_ppsSize) & 0xff;
    memcpy(&buffer[i], m_pps, m_ppsSize);
    return bufferSize;
}

int RtmpSenderPlugin::BodyToBuffer(char *nalData, int size, bool isKeyFrame, char **outBuffer) {
    *outBuffer = NULL;
    if (!m_pps || !m_sps || !nalData) {
        LogE("NalToBuffer nal=%p, pps_=%p, sps_=%p", nalData, m_pps, m_sps);
        return 0;
    }

    if (nalData[2] == 0x00) {
        nalData += 4;
        size -= 4;
    }
    else if (nalData[2] == 0x01) {
        nalData += 3;
        size -= 3;
    }
    char *buffer = *outBuffer = (char*)malloc(size + 9);
    if (isKeyFrame) {
        buffer[0] = 0x17;
    }
    else {
        buffer[0] = 0x27;
    }

    buffer[1] = 0x01;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    buffer[5] = (size >> 24) & 0xff;
    buffer[6] = (size >> 16) & 0xff;
    buffer[7] = (size >> 8) & 0xff;
    buffer[8] = (size) & 0xff;

    memcpy(&buffer[9], nalData, size);
    return size + 9;
}

RtmpSenderPlugin::~RtmpSenderPlugin() {
    if (m_pps) {
        free(m_pps);
        m_pps = NULL;
    }

    if (m_sps) {
        free(m_sps);
        m_sps = NULL;
    }
}

PluginBase* GetRtmpSenderInstance() {
    PluginBase* pInstance = new  RtmpSenderPlugin();
    return pInstance;
}
