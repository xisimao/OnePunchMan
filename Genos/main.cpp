#include "FlowStartup.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    if (argc > 1)
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
        else if (arg.compare("debug") == 0)
        {
            DecodeChannel::InitFFmpeg();
            DecodeChannel::UninitHisi(FlowChannelData::ChannelCount);
            if (!DecodeChannel::InitHisi(FlowChannelData::ChannelCount))
            {
                return -1;
            }
            if (!SeemmoSDK::Init())
            {
                return -1;
            }
            int channelIndex = 1;
            if (argc > 2)
            {
                channelIndex = StringEx::Convert<int>(argv[2]);
            }
            ChannelDetector detector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, NULL, true);
            FlowChannelData data;
            FlowChannel channel = data.Get(channelIndex);
            detector.UpdateChannel(channel);
            vector<ChannelDetector*> detectors;
            detectors.push_back(&detector);
            RecognChannel recogn(0, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, detectors);
            DetectChannel detect(1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, &recogn, &detector);
            DecodeChannel decode(channel.ChannelUrl, string(), channel.ChannelIndex, &detect,true);
            recogn.Start();
            detect.Start();
            decode.Start();
            decode.Join();
            detect.Stop();
            recogn.Stop();
            SeemmoSDK::Uninit();
            DecodeChannel::UninitHisi(FlowChannelData::ChannelCount);
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

    return 0;
}