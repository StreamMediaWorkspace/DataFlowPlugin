#pragma once
#include <comdef.h>
#include <string>

namespace Utils
{
    //��wstringת����string  
    std::string WString2String(std::wstring wstr);
    std::wstring String2WString(std::string str);
    std::string HResultToString(HRESULT hr);

    long GetTimeStamp();
};

