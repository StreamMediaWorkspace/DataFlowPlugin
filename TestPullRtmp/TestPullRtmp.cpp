// TestPullRtmp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "RtmpReceiver.h"
int main() {
    RtmpReceiver test;
//     for (int i = 0; i < 3; i++) {
//         test.Start("rtmp://101.132.187.172/live1/xyz111222");
//         Sleep(3000);
//     }
    test.Start("rtmp://101.132.187.172/live1/xyz111222");
    Sleep(10000);
    test.Stop();
    system("pause");
    
    return 0;
}

