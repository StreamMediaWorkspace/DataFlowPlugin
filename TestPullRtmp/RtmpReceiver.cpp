#include "stdafx.h"
#include "RtmpReceiver.h"

#include <time.h>
#include <stdlib.h>
#pragma comment(lib, "../DataFlowPlugin/lib/librtmp.lib")
#pragma comment(lib,"WS2_32.lib") 

RtmpReceiver::RtmpReceiver() {
    InitSockets();
}

RtmpReceiver::~RtmpReceiver() {
    Stop();
    CleanupSockets();
}

void RtmpReceiver::Deleter::operator()(RtmpReceiverImpl *pImpl) {
    if (pImpl) {
        pImpl->Stop();
        delete pImpl;
        pImpl = nullptr;
    }
}

int RtmpReceiver::Start(const char *url) {
    m_impl.reset(new RtmpReceiverImpl());
    return m_impl->Start(url);
}

int RtmpReceiver::Stop() {
    if (m_impl) {
        m_impl->Stop();
        m_impl.reset();
    }
    return 0;
}

int RtmpReceiver::InitSockets() {
#ifdef WIN32
    WORD version;
    WSADATA wsaData;
    version = MAKEWORD(2, 2);
    return (WSAStartup(version, &wsaData) == 0);
#endif
}

void RtmpReceiver::CleanupSockets() {
#ifdef WIN32
    WSACleanup();
#endif
}

RtmpReceiverImpl::RtmpReceiverImpl() {
}

int RtmpReceiverImpl::Stop() {
    m_stop = true;
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
    return 0;
}

int RtmpReceiverImpl::Loop(void *p) {
    const char *url = (const char *)p;
    RTMP *pRtmp = RTMP_Alloc();
    RTMP_Init(pRtmp);
    pRtmp->Link.timeout = 5;
    if (!RTMP_SetupURL(pRtmp, (char*)url))
    {
        RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
        RTMP_Free(pRtmp);
        return -1;
    }

    //if unable,the AMF command would be 'play' instead of 'publish'
    //RTMP_EnableWrite(m_pRtmp);
    RTMP_SetBufferMS(pRtmp, 10000 * 1000);
    if (!RTMP_Connect(pRtmp, NULL)) {
        RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
        RTMP_Free(pRtmp);
        pRtmp = NULL;
        return -1;
    }

    if (!RTMP_ConnectStream(pRtmp, 0)) {
        RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
        RTMP_Close(pRtmp);
        RTMP_Free(pRtmp);
        pRtmp = NULL;
        return -1;
    }

    //À­Á÷
    int nRead = 0, NRead = 0;
    int bufsize = 1024 * 1024;
    char* buf = (char*)malloc(bufsize);
    FILE* fp_save = nullptr;
    fopen_s(&fp_save, "./save.flv", "wb");
    if (!fp_save) {
        LogI("Open file to write failed\n");
        return -1;
    }

    m_stop = false;
    while (!m_stop) {
        if (nRead = RTMP_Read(pRtmp, buf, bufsize)) {
            fwrite(buf, 1, nRead, fp_save);
            NRead += nRead;
            RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB\n", nRead, NRead*1.0 / 1024);
        } else {
            LogE("RtmpReceiverImpl::Loop %p error\n", this);
        }
    }
    fclose(fp_save);
    fp_save = nullptr;
    RTMP_Close(pRtmp);
    RTMP_Free(pRtmp);
}

int RtmpReceiverImpl::Start(const char *url) {
    if (m_stop == false) {
        return 0;
    }

    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }

    LogI("RtmpReceiverImpl::Start %p enter %s\n", this, url);

    m_thread.reset(new std::thread(&RtmpReceiverImpl::Loop, this, (void*)url));
    return 0;
}

RtmpReceiverImpl::~RtmpReceiverImpl() {
    Stop();
}
