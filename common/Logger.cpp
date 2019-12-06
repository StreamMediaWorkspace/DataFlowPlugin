#pragma once
#include "logger.h"

static void OutputDebugStrImpl(char *Msg, int len, const char *fmt, va_list _ArgList) {
    std::string logMsg;
    std::string levelMsg;
    levelMsg = "D";

    vsnprintf(Msg, len, fmt, _ArgList);
#ifdef _DEBUG
    //OutputDebugStringA(Msg);
#else
    if (log2console_flags) {
        OutputDebugStringA(Msg);
    }
#endif

    logMsg = Msg;
    static FILE *f = nullptr;
    if (!f) {
        fopen_s(&f, "./log.txt", "a+");
    }

    fwrite(logMsg.c_str(), 1, logMsg.length()+1, f);
}

void Log(const char *fmt, ...) {
    char tmpString[1024] = { 0 };
    va_list ap;
    va_start(ap, fmt);
    OutputDebugStrImpl(tmpString, sizeof(tmpString) - 1, fmt, ap);
    va_end(ap);
}
