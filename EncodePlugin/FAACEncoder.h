#pragma once
#include "include/faac/faac.h"
#pragma comment(lib, "../DataFlowPlugin/lib/libfaac.lib")

class FAACEncoder
{
public:
    FAACEncoder();
    ~FAACEncoder();

    void Init(unsigned int samRate, unsigned int channels, int bitsPerSample);
    void Encode(unsigned char* inputBuf, unsigned int samCount, unsigned char** outBuf, unsigned int& bufSize);
    unsigned long GetMaxInputSamples();
    unsigned long GetMaxOutputBytes();

    int GetDecoderSpecificInfo(char ** outBuffer);
private:
    faacEncHandle faac_handle_;
    unsigned long input_sams_;
    unsigned long max_output_bytes_;
};

