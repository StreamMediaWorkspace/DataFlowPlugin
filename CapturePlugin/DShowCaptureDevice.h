#pragma once
#include <atlcomcli.h>
#include <control.h>
#include <strmif.h>
#include <list>

#include <qedit.h>
#include <map>
#include <vector>
#include <string>

class DShowCaptureDevice
{
public:
    static void ListCapDevices(const IID& deviceIID, std::map<std::string, std::string>& deviceList);

    static void ListVideoCapDeviceWH(const std::string& vDeviceID,
        std::vector<int>& widthList, std::vector<int>& heightList);

public:
    DShowCaptureDevice();
    virtual ~DShowCaptureDevice();
};

