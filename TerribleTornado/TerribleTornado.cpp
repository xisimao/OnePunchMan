#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "YUV420PHandler.h"
#include "EncodeHandler.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    FFmpegChannel::InitFFmpeg();
    FFmpegChannel channel(false);
    channel.UpdateChannel("600w.mp4", string());
    channel.Start();
    system("pause");
    channel.Stop();
    return 0;
}