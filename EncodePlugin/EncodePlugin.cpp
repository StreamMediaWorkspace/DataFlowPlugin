// EncodePlugin.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "EncodePlugin.h"
#include "../common/Logger.h"
#include "CYUVTrans.h"

EncodePlugin::EncodePlugin() {

}

EncodePlugin::~EncodePlugin() {
}

int EncodePlugin::Start() {
    //your code
    if (m_pThread) {
        delete m_pThread;
        m_pThread = NULL;
    }

    m_stop = false;
    m_pThread = new std::thread(EncodeThread, this);
    return PluginBase::Start();
}

int EncodePlugin::Stop() {
    //your code
    m_stop = true;
    if (m_pThread->joinable()) {
        m_pThread->join();
    }
    return PluginBase::Stop();
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

    if (metaData.getProperty(ENCODE_PLUGIN_KEY_BITRATE, value) == 0) {
        m_nBitrate = (int)value;
    }

    if (m_nWidth > 0 && m_nHeight > 0 && m_nFps > 0 && m_nBitrate > 0) {
        m_X246Encode.Initialize(m_nWidth, m_nHeight, m_nBitrate, m_nFps);
        ret = 0;
        LogI("EncodePlugin::Control width=%d,height=%d,fps=%d\n", m_nWidth, m_nHeight, m_nFps);
    }
    PluginBase::Control(metaData);
    return ret;
}

void EncodePlugin::Loop() {
    unsigned int lastKeyFrameTimestamp = 0;
    DataBuffer *pDataBuffer = NULL;
    m_X246Encode.Initialize(m_nWidth, m_nHeight, m_nFps);
    //FILE* infile = NULL;
    //fopen_s(&infile, "./176_144_420planar.yuv", "rb");

    while (!m_stop) {
        {
            pDataBuffer = m_dataBufferQueue.pop();
        }

        if (pDataBuffer) {
            int width = pDataBuffer->Width();
            int height = pDataBuffer->Height();
            //SaveRgb2Bmp(pDataBuffer->Buf(), width, height);//RGB24

            unsigned long yuvimg_size = width * height * 3 / 2;
            unsigned char* yuvbuf = (unsigned char*)malloc(yuvimg_size);
            unsigned char* x264buf = NULL;
            int x264buf_len = yuvimg_size * 100;

            //GBK24
            //             CYUVTrans::RGB24ToI420((LPBYTE)pDataBuffer->Buf(), (LPBYTE)yuvbuf,
            //                 yuvimg_size, width, height);

            FILE *f1 = NULL;
            fopen_s(&f1, "./test1.yuv", "wb");
            fwrite(pDataBuffer->Buf(), pDataBuffer->BufLen(), 1, f1);
            fclose(f1);

            //YUY2
            //              CYUVTrans::YUY2ToI420((LPBYTE)pDataBuffer->Buf(), (LPBYTE)yuvbuf,
            //                  yuvimg_size, width, height);
            BOOL ret = CYUVTrans::YUV422To420((LPBYTE)pDataBuffer->Buf(), yuvbuf, yuvbuf + width * height,
                yuvbuf + width * height + width * height / 4, width, height);

            //             static FILE *fp = NULL;
            //             if (!fp) {
            //                 fopen_s(&fp, "./test.yuv420", "wb");
            //             }
            //             fwrite(yuvbuf, yuvimg_size, 1, fp);
            //             fflush(fp);
            bool is_keyframe = false;
            if (GetTickCount() - lastKeyFrameTimestamp > 5000) {
                is_keyframe = true;
            }
            x264_nal_t *nals = NULL;
            int nalCount = 0;
            if (0 != m_X246Encode.Encode((unsigned char*)yuvbuf, pDataBuffer->Width(),
                pDataBuffer->Height(),
                nals, nalCount, is_keyframe)) {
                LogE("GetPPSFromNals failed\n");
                if (yuvbuf) {
                    free(yuvbuf);
                }
                yuvbuf = NULL;
                continue;
            }

            if (is_keyframe) {
                lastKeyFrameTimestamp = GetTickCount();
                is_keyframe = false;
            }
            X264DataBuffer::Type type;
            int size = 0;
            char *buffer = NULL;
            for (int i = 0; i < nalCount; i++) {
                x264_nal_t *nal = &nals[i];
                if (nal->i_type == NAL_SPS) {
                    size = nal->i_payload - 4;
                    buffer = (char*)malloc(size);
                    memcpy(buffer, nal->p_payload + 4, size);
                    type = X264DataBuffer::euSPS;
                }
                else if (nal->i_type == NAL_PPS) {
                    size = nal->i_payload - 4;
                    buffer = (char*)malloc(size);
                    memcpy(buffer, nal->p_payload + 4, size);
                    type = X264DataBuffer::euPPS;
                }
                else {
                    size = nal->i_payload;
                    buffer = (char*)malloc(size);
                    memcpy(buffer, nal->p_payload, size);
                    type = X264DataBuffer::euBody;
                }
                PluginBase::Input(new X264DataBuffer((unsigned char*)buffer, size, width, height, pDataBuffer->TimeStamp(), type, nal->i_type == NAL_SLICE_IDR));
            }

            free(yuvbuf);
            yuvbuf = NULL;
            delete pDataBuffer;
            pDataBuffer = NULL;
        }
    }
}

void EncodePlugin::EncodeThread(EncodePlugin *pEncodePlugin) {
    if (pEncodePlugin) {
        pEncodePlugin->Loop();
    }
}

void EncodePlugin::Input(DataBuffer *pDataBuffer) {
    LogI("EncodePlugin::Input data len=%d\n", pDataBuffer->BufLen());
    m_dataBufferQueue.push(pDataBuffer);
}

void EncodePlugin::SaveRgb2Bmp(char* rgbbuf, unsigned int width, unsigned int height) {
    BITMAPINFO bitmapinfo;
    ZeroMemory(&bitmapinfo, sizeof(BITMAPINFO));
    bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapinfo.bmiHeader.biWidth = width;
    bitmapinfo.bmiHeader.biHeight = -1 * height;
    bitmapinfo.bmiHeader.biPlanes = 1;
    bitmapinfo.bmiHeader.biBitCount = 24;
    bitmapinfo.bmiHeader.biXPelsPerMeter = 0;
    bitmapinfo.bmiHeader.biYPelsPerMeter = 0;
    bitmapinfo.bmiHeader.biSizeImage = width*height;
    bitmapinfo.bmiHeader.biClrUsed = 0;
    bitmapinfo.bmiHeader.biClrImportant = 0;

    BITMAPFILEHEADER bmpHeader;
    ZeroMemory(&bmpHeader, sizeof(BITMAPFILEHEADER));
    bmpHeader.bfType = 0x4D42;
    bmpHeader.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
    bmpHeader.bfSize = bmpHeader.bfOffBits + width*height * 3;

    FILE* fp = NULL;
    fopen_s(&fp, "./CameraCodingCapture.bmp", "wb");
    if (fp) {
        fwrite(&bmpHeader, 1, sizeof(BITMAPFILEHEADER), fp);
        fwrite(&(bitmapinfo.bmiHeader), 1, sizeof(BITMAPINFOHEADER), fp);
        fwrite(rgbbuf, 1, width * height * 3, fp);
        fclose(fp);
    }
}

PluginBase* GetEncodeInstance() {
    PluginBase* pInstance = new  EncodePlugin();
    return pInstance;
}
