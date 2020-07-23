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
    channel.UpdateChannel("600w.mp4", string(), true);
    channel.Start();
    system("pause");

    return 0;
}
