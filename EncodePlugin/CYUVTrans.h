#pragma once
class CYUVTrans
{
public:
    CYUVTrans();
    ~CYUVTrans();

    static BOOL I420ToRGB24(const LPBYTE lpYUVBuffer, LPBYTE lpBGR24Buffer, SIZE_T nRGBBufSize, int nWidth, int nHeight);

    static BOOL RGB24ToI420(const LPBYTE lpBGR24Buffer, LPBYTE lpYUVBuffer, SIZE_T nYUVBufSize, int nWidth, int nHeight);

    static BOOL YUY2ToI420(const LPBYTE lpYUY2Buffer, LPBYTE lpYUVBuffer, SIZE_T nYUVBufSize, int nWidth, int nHeight);

    static BOOL UYVYToI420(const LPBYTE lpUYVYBuffer, LPBYTE lpYUVBuffer, SIZE_T nYUVBufSize, int nWidth, int nHeight);

    static BOOL YUV422To420(BYTE* pYUV, BYTE* pY, BYTE* pU, BYTE* pV, LONG lWidth, LONG lHeight);
private:
    static const int m_nfv1Table[256];
    static const int m_nfv2Table[256];
    static const int m_nfu1Table[256];
    static const int m_nfu2Table[256];
};

