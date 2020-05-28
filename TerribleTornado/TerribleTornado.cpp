#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    FFmpegChannel::InitFFmpeg();
    FFmpegChannel channel("2.mp4", string(), false);
    channel.Start();
    channel.Join();

    //FlowStartup channel;
    //channel.Startup();
    return 0;
}