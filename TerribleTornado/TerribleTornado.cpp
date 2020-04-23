#include <stdio.h>
#include <stdlib.h>
#include "mosquitto.h"
#include <string.h>
#include "LogPool.h"
#include <string>
#include <iostream>

#include "DataChannel.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;
using namespace TerribleTornado;

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        string arg(argv[1]);
        if (arg.compare("stop") == 0)
        {
            SocketMaid maid(2);
            SocketHandler handler;
            maid.AddConnectEndPoint(EndPoint("192.168.201.139", 7772), &handler);
            maid.Start();
            this_thread::sleep_for(chrono::seconds(5));
            stringstream ss;
            ss << "GET /api/system HTTP/1.1\r\n"
                << "\r\n"
                << "\r\n";
            maid.SendTcp(EndPoint("192.168.201.139", 7772), ss.str());
        }
    }
    else
    {
        DataChannel channel;
        channel.Start();
        channel.Join();
    }

    return 0;
}

//int main3()
//{
//    if (SeemmoSDK::Init())
//    {
//        RecognChannel recogn(1920, 1080);
//        DetectChannel detect(1920, 1080, &recogn);
//
//        int result = detect.InitUrl("/mtd/seemmo/ncdj1.mp4");
//        if (result == 0)
//        {
//            detect.Start();
//            recogn.Start();
//            while (true)
//            {
//                this_thread::sleep_for(chrono::milliseconds(1000));
//            }
//            detect.Stop();
//            recogn.Stop();
//
//        }
//        SeemmoSDK::Uninit();
//    }
//    return 0;
//}
