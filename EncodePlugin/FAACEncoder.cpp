#include "stdafx.h"
#include <memory>
#include "FAACEncoder.h"


FAACEncoder::FAACEncoder() {
}

void FAACEncoder::Init(unsigned int samRate, unsigned int channels, int bitsPerSample)
{
    faac_handle_ = faacEncOpen(samRate, channels, &input_sams_, &max_output_bytes_);
    faacEncConfigurationPtr enc_cfg = faacEncGetCurrentConfiguration(faac_handle_);
    switch (bitsPerSample) {
    case 16:
        enc_cfg->inputFormat = FAAC_INPUT_16BIT;
        break;
    case 24:
        enc_cfg->inputFormat = FAAC_INPUT_24BIT;
        break;
    case 32:
        enc_cfg->inputFormat = FAAC_INPUT_32BIT;
        break;
    default:
        enc_cfg->inputFormat = FAAC_INPUT_NULL;
        break;
    }

    enc_cfg->outputFormat = 1;
    //enc_cfg->mpegVersion = MPEG4;
//    enc_cfg->aacObjectType = LOW;
//     enc_cfg->allowMidside = 1;
//     enc_cfg->useLfe = 0;
//     enc_cfg->useTns = 0;
//     enc_cfg->bitRate = 4800;
//     enc_cfg->quantqual = 100;
//     enc_cfg->bandWidth = 32000;
//     enc_cfg->outputFormat = 0;
    int ret = faacEncSetConfiguration(faac_handle_, enc_cfg);
}

unsigned long FAACEncoder::GetMaxInputSamples() {
    return input_sams_;
}

unsigned long FAACEncoder::GetMaxOutputBytes() {
    return max_output_bytes_;
}

int FAACEncoder::GetDecoderSpecificInfo(char ** outBuffer) {
    unsigned long len;
    int i = faacEncGetDecoderSpecificInfo(faac_handle_, (unsigned char**)outBuffer, &len);
    return len;
}

void FAACEncoder::Encode(unsigned char* inputBuf, unsigned int samCount, unsigned char** outBuf, unsigned int& bufSize) {
    *outBuf = (unsigned char*)calloc(max_output_bytes_, 1);
    char *buffer = (char*)(*outBuf);
    bufSize = faacEncEncode(faac_handle_, (int*)inputBuf, samCount, (unsigned char*)buffer, max_output_bytes_);
}

FAACEncoder::~FAACEncoder() {
    faacEncClose(faac_handle_);
}
