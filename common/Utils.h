#pragma once
#include <comdef.h>
#include <string>
namespace Utils
{
    //½«wstring×ª»»³Éstring  
    std::string WString2String(std::wstring wstr);

    std::string HResultToString(HRESULT hr);
};

