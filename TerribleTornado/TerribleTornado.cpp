#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    unsigned char* bgrBuffer = new unsigned char[1920*1080*3];
    int size = tjBufSize(1920, 1080, TJSAMP_422);
    unsigned char* jpgBuffer=tjAlloc(size);
    while (true)
    {
        TrafficDetector::BgrToJpg(bgrBuffer, 1920, 1080, &jpgBuffer,size);
    }
  
    LogPool::Init("appsettings.json");
    FFmpegChannel::InitFFmpeg();
    DecodeChannel channel(1,false);
    channel.UpdateChannel("600w.mp4", "rtmp://192.168.201.139:1935/live/7");
    channel.Start();
    system("pause");
    channel.Stop();
    return 0;
}