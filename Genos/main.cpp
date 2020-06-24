#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "HisiEncodeChannel.h"

using namespace std;
using namespace OnePunchMan;

int main1()
{
    LogPool::Init("appsettings.json");
    HisiDecodeChannel::InitFFmpeg();
    HisiDecodeChannel::UninitHisi(8);
    HisiDecodeChannel::InitHisi(8);
    HisiEncodeChannel encode(8);
    HisiDecodeChannel channel(1,&encode);
    channel.UpdateChannel("/root/service/video/600w.mp4", string(), true);

    encode.Start();
    channel.Start();
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1));
    }
    return 0;
}
int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("flow") == 0)
        {
            LogPool::Init("appsettings.json");
            FlowStartup channel;
            channel.Startup();
        }
        else if (arg.compare("event") == 0)
        {
            LogPool::Init("appsettings.json");
            EventStartup channel;
            channel.Startup();
        }
        else if (arg.compare("io") == 0)
        {
            LogPool::Init("io.json");
            IoAdapter adapter;
            adapter.Start();
            adapter.Join();
        }
    }
    return 0;
}