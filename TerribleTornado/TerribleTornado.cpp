﻿#include "FlowStartup.h"
#include "EventStartup.h"

using namespace std;
using namespace OnePunchMan;

class Test:public ThreadObject
{
public:

    Test()
        :ThreadObject("1")
    {

    }

    void Fun()
    {
        
        unique_lock<timed_mutex> ll(_mutex, std::defer_lock);
        if (ll.try_lock_for(chrono::seconds(3)))
        {
            cout << "ok" << endl;
            ll.unlock();
        }
        else
        {
            cout << "fuck" << endl;

        }
     
    }
protected:

    void StartCore()
    {
        while (!_cancelled)
        {
            unique_lock<timed_mutex> ll(_mutex, std::defer_lock);
            if (ll.try_lock_for(chrono::seconds(3)))
            {
                cout << "1" << endl;
                this_thread::sleep_for(chrono::seconds(10));
                cout << "2" << endl;
            }
            else
            {
                cout << "fuck" << endl;

            }
            ll.unlock();
            this_thread::sleep_for(chrono::seconds(3));
        }
    }
private:
    timed_mutex _mutex;
};

int main()
{
    Test s;
    s.Start();
    system("pause");
    s.Fun();
    s.Stop();


    return 0;
}
int main1(int argc, char* argv[])
{
    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("stop") == 0)
        {
            SocketMaid maid(2);
            SocketHandler handler;
            maid.AddConnectEndPoint(EndPoint("192.168.201.139", 7772), &handler);
            maid.Start();
            this_thread::sleep_for(chrono::seconds(5));
            stringstream ss;
            ss << "GET /api/system HTTP/1.1\r\n"
                << "\r\n"
                << "\r\n";
            maid.SendTcp(EndPoint("192.168.201.139", 7772), ss.str());
        }
        else if (arg.compare("debug") == 0)
        {
            FFmpegChannel::InitFFmpeg();
            DecodeChannel::UninitHisi(FlowStartup::ChannelCount);
            if (!DecodeChannel::InitHisi(FlowStartup::ChannelCount))
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
            FlowDetector detector(FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, NULL,true);
            FlowChannelData data;
            FlowChannel channel = data.Get(channelIndex);
            detector.UpdateChannel(channel);
            vector<TrafficDetector*> detectors;
            detectors.push_back(&detector);
            RecognChannel recogn(0, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, detectors);
            DetectChannel detect(1, FFmpegChannel::DestinationWidth, FFmpegChannel::DestinationHeight, &recogn, &detector);
            DecodeChannel decode(channel.ChannelUrl,string(), channel.ChannelIndex, &detect, true);
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

    return 0;
}