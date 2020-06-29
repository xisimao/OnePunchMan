#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "HisiEncodeChannel.h"
#include "OutputHandler.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    FlowStartup startup;
    startup.Startup();
    //DecodeChannel::InitFFmpeg();

    //HisiDecodeChannel channel(1);
    //channel.UpdateChannel("600w.mp4", string(), false);
    //HisiEncodeChannel encode;
    //encode.Start();
    //channel.Start();
    //this_thread::sleep_for(chrono::seconds(3));
    //OutputHandler handler;
    //handler.Init(1,"temp.mp4", "600w.mp4",250);
    //unsigned char* data = new unsigned char [1024 * 1024];
    //for (int i =1; i <=100; ++i)
    //{
    //    FILE* file = fopen(StringEx::Combine("../images/h264_", i, ".h264").c_str(), "rb");
    //    size_t size=fread(data, 1, 1024 * 1024, file);
    //    handler.PushPacket(data, size);
    //    fclose(file);
    //}
    //handler.Uninit();
    //while (true)
    //{
    //    this_thread::sleep_for(chrono::seconds(1));
    //}
    system("pause");
    //channel.Stop();
    return 0;
}