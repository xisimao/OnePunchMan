#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
	
    LogPool::Init("appsettings.json");
  

    FlowStartup startup;
    startup.Startup();
    /*FFmpegChannel::InitFFmpeg();
    DecodeChannel channel(1);
    channel.UpdateChannel("600w.mp4", "rtmp://192.168.201.139:1935/live/7",false);
    channel.Start();
    system("pause");
    channel.Stop();*/
    return 0;
}