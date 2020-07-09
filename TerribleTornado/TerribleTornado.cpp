#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "HisiEncodeChannel.h"
#include "OutputHandler.h"
#include "DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    DecodeChannel::InitFFmpeg();
    DecodeChannel channel(1);
    channel.UpdateChannel("600w.mp4", "rtmp://192.168.201.144:1935/live/1",true);
    channel.Start();
    system("pause");
    channel.Stop();
    return 0;
}