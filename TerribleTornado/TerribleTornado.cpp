#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    FFmpegChannel::InitFFmpeg();
    char buffer[1024] = { 0 };
    av_strerror(-110, buffer, 1024);
    FlowStartup channel;
    channel.Startup();
    return 0;
}