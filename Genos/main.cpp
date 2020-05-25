#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("flow") == 0)
        {
            LogPool::Init("appsettings.json");
            if (argc >= 3)
            {
                string mode(argv[2]);
                if (mode.compare("debug") == 0)
                {
                    DecodeChannel::InitFFmpeg();
                    DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
                    if (!DecodeChannel::InitHisi(FlowStartup::ChannelCount))
                    {
                        return -1;
                    }
                    if (!SeemmoSDK::Init())
                    {
                        return -1;
                    }
                    FlowDetector detector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight,NULL, true);
                    FlowChannelData data;
                    int channelIndex = 1;
                    if (argc >= 4)
                    {
                        channelIndex = StringEx::Convert<int>(argv[3]);
                    }
                    FlowChannel channel = data.Get(channelIndex);
                    detector.UpdateChannel(channel);
                    DetectChannel detect(1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, NULL, &detector);
                    DecodeChannel decode(channel.ChannelUrl, string(), channel.ChannelIndex, &detect, true);
                    detect.Start();
                    decode.Start();
                    decode.Join();
                    detect.Stop();
                    SeemmoSDK::Uninit();
                    DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
                    DecodeChannel::UninitFFmpeg();
                    LogPool::Uninit();
                }
            }
            else
            {
                FlowStartup channel;
                channel.Startup();
            }
        }
        else if (arg.compare("event") == 0)
        {
            LogPool::Init("appsettings.json");
            if (argc >= 3)
            {
                string mode(argv[2]);
                if (mode.compare("debug") == 0)
                {
                    DecodeChannel::InitFFmpeg();
                    DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
                    if (!DecodeChannel::InitHisi(FlowStartup::ChannelCount))
                    {
                        return -1;
                    }
                    if (!SeemmoSDK::Init())
                    {
                        return -1;
                    }
                    EventDetector detector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight,NULL, true);
                    EventChannelData data;
                    int channelIndex = 1;
                    if (argc >= 4)
                    {
                        channelIndex = StringEx::Convert<int>(argv[3]);
                    }
                    EventChannel channel = data.Get(channelIndex);
                    detector.UpdateChannel(channel);
                    DetectChannel detect(1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, NULL, &detector);
                    DecodeChannel decode(channel.ChannelUrl, string(), channel.ChannelIndex, &detect, true);
                    detect.Start();
                    decode.Start();
                    decode.Join();
                    detect.Stop();
                    SeemmoSDK::Uninit();
                    DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
                    DecodeChannel::UninitFFmpeg();
                    LogPool::Uninit();
                }
            }
            else
            {
                EventStartup channel;
                channel.Startup();
            }
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