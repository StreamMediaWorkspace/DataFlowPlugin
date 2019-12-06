#pragma once
#include <memory>
#include <thread>
#include <atomic>

#include "../RtmpPlugin/include/librtmp/rtmp_sys.h"
#include "../RtmpPlugin/include/librtmp/log.h"
#include "../RtmpPlugin/include/x264/x264.h"
#include "../RtmpPlugin/include/x264/x264_config.h"
#include "../common/Logger.h"

class RtmpReceiverImpl;

class RtmpReceiver {
public:
    RtmpReceiver();
    ~RtmpReceiver();

    int Start(const char *url);
    int Stop();

private:
    int InitSockets();
    void CleanupSockets();

    struct Deleter {
        void operator()(RtmpReceiverImpl *pImpl);
    };

private:
    std::unique_ptr<RtmpReceiverImpl, Deleter> m_impl = nullptr;
};

class RtmpReceiverImpl
{
public:
    RtmpReceiverImpl();
    ~RtmpReceiverImpl();

    int Start(const char *url);
    int Stop();

protected:
    int Loop(void *p);

private:
    struct Deleter{
        void operator() (std::thread* pThread) {
            if (pThread && pThread->joinable()) {
                pThread->join();
            }
            delete pThread;
            pThread = nullptr;
            LogI("RtmpReceiverImpl::ThreadDeleter %p\n", this);
        }
    };

private:
    std::unique_ptr<std::thread, Deleter> m_thread = nullptr;
    std::atomic_bool m_stop = true;
};

