#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"
#include "SqliteLogger.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    TrafficDirectory::Init(jd);
    TrafficData::Init(jd.Get<string>("Flow:Db"));
    SqliteLogger logger(TrafficData::DbName);
    LogPool::AddLogger(&logger);
    DecodeChannel::InitFFmpeg();
    DecodeChannel channel(1, -1, NULL);
    channel.UpdateChannel("rtsp://admin:Hikjxjj123@66.66.12.86/media/video1/multicast", "", ChannelType::RTSP, true);
    channel.Start();
    channel.Join();

    return 0;
}
