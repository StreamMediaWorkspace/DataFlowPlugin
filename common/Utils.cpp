#include "Utils.h"
#include <chrono>


//��wstringת����string  
std::string Utils::WString2String(std::wstring wstr)
{
    std::string result;
    //��ȡ��������С��������ռ䣬��������С�°��ֽڼ����  
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buffer = new char[len + 1];
    //���ֽڱ���ת���ɶ��ֽڱ���  
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    //ɾ��������������ֵ  
    result.append(buffer);
    delete[] buffer;
    return result;
}

std::wstring Utils::String2WString(std::string str) {
    std::wstring result; 
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    TCHAR* buffer = new TCHAR[len + 1];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
    buffer[len] = '\0'; 
    result.append(buffer);
    delete[] buffer;
    return result;
}

std::string Utils::HResultToString(HRESULT hr) {
    _com_error err(hr);
    return WString2String(err.ErrorMessage());
}


long Utils::GetTimeStamp()
{
    return GetTickCount();
}
