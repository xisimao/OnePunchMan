#include "FlowStartup.h"
#include "EventStartup.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("stop") == 0)
        {
            SocketMaid maid(2);
            SocketHandler handler;
            maid.AddConnectEndPoint(EndPoint("127.0.0.1", 7772), &handler);
            maid.Start();
            this_thread::sleep_for(chrono::seconds(3));
            stringstream ss;
            ss << "GET /api/system HTTP/1.1\r\n"
                << "\r\n"
                << "\r\n";
            maid.SendTcp(EndPoint("127.0.0.1", 7772), ss.str());
        }
        else if (arg.compare("flow") == 0)
        {
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
                    FlowDetector detector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, NULL, true);
                    FlowChannelData data;
                    FlowChannel channel = data.Get(1);
                    detector.UpdateChannel(channel);
                    vector<TrafficDetector*> detectors;
                    detectors.push_back(&detector);
                    RecognChannel recogn(0, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, detectors);
                    DetectChannel detect(1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, &recogn, &detector);
                    DecodeChannel decode(channel.ChannelUrl, string(), channel.ChannelIndex, &detect, true);
                    recogn.Start();
                    detect.Start();
                    decode.Start();
                    decode.Join();
                    detect.Stop();
                    recogn.Stop();
                    SeemmoSDK::Uninit();
                    DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
                    DecodeChannel::UninitFFmpeg();
                }
            }
            else
            {
                FlowStartup channel;
                if (channel.Init())
                {
                    channel.Start();
                    channel.Join();
                }
                else
                {
                    LogPool::Information("init flow system failed");
                }
            }
        }
        else if (arg.compare("event") == 0)
        {
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
                    EventDetector detector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, NULL, true);
                    EventChannelData data;
                    EventChannel channel = data.Get(1);
                    detector.UpdateChannel(channel);
                    vector<TrafficDetector*> detectors;
                    detectors.push_back(&detector);
                    RecognChannel recogn(0, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, detectors);
                    DetectChannel detect(1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, &recogn, &detector);
                    DecodeChannel decode(channel.ChannelUrl, string(), channel.ChannelIndex, &detect, true);
                    recogn.Start();
                    detect.Start();
                    decode.Start();
                    decode.Join();
                    detect.Stop();
                    recogn.Stop();
                    SeemmoSDK::Uninit();
                    DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
                    DecodeChannel::UninitFFmpeg();
                }
            }
            else
            {
                EventStartup channel;
                if (channel.Init())
                {
                    channel.Start();
                    channel.Join();
                }
                else
                {
                    LogPool::Information("init event system failed");
                }
            }
        }
    }
    return 0;
}