#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    int frameIndex = 12345;
    unsigned char taskId = 5;
    unsigned char frameSpan = 40;
    unsigned long long temp = frameIndex;
    unsigned long long timeStamp = 0;
    timeStamp |= temp;
    temp = taskId;
    timeStamp |= (temp << 32);
    temp = frameSpan;
    timeStamp |= (temp << 40);
    unsigned char taskId1 = timeStamp >> 32 & 0xFF;
    unsigned char frameSpan1 = timeStamp >> 40 & 0xFF;
    unsigned int frameIndex1 = timeStamp & 0xFFFFFFFF;

     //FlowStartup startup;
    //startup.Startup();
    /*FFmpegChannel::InitFFmpeg();
    DecodeChannel channel(1);
    channel.UpdateChannel("600w.mp4", "rtmp://192.168.201.139:1935/live/7",false);
    channel.Start();
    system("pause");
    channel.Stop();*/
    return 0;
}