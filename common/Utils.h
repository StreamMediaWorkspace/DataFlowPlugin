#pragma once
#include <comdef.h>
#include <string>
namespace Utils
{
    //��wstringת����string  
    std::string WString2String(std::wstring wstr);

    std::string HResultToString(HRESULT hr);
};

