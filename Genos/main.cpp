//#include "DataChannel.h"
//#include "SocketMaid.h"
//#include "HttpHandler.h"
//
//using namespace std;
//using namespace Saitama;
//
//int main()
//{
//    SocketMaid maid(2);
//    DataChannel channel("127.0.0.1", 1884);
//    HttpHandler handler;
//    handler.HttpReceived.Subscribe(&channel);
//    maid.AddListenEndPoint(EndPoint(7772), &handler);
//    channel.Start();
//    maid.Start();
//
//    while (true)
//    {
//        this_thread::sleep_for(chrono::seconds(1));
//    }
//
//    maid.Stop();
//    channel.Stop();
//     
//    return 0;
//}
#include <iostream>

#include "HisiChannel.h"
#include "DetectChannel.h"
#include "StringEx.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;
using namespace TerribleTornado;

int main()
{
    int videoCount = 8;
    HisiChannel::InitFFmpeg();
    HisiChannel::UninitHisi(videoCount);
    HisiChannel::InitHisi(videoCount);
    SeemmoSDK::Init();

    MqttChannel mqtt("127.0.0.1", 1883);
    mqtt.Start();

    FlowChannelData data;
    vector<FlowChannel> channels = data.GetList();

    vector<RecognChannel*> recogns;
    for (int i = 0; i < videoCount / RecognChannel::ItemCount; ++i)
    {
        vector<LaneDetector*> lanes;
        for (vector<Lane>::iterator it = channels[i].Lanes.begin(); it != channels[i].Lanes.end(); ++it)
        {
            lanes.push_back(new LaneDetector(StringEx::Combine(channels[i].ChannelUrl, "_", it->LaneId), *it));
        }
        RecognChannel* recogn = new RecognChannel(i, HisiChannel::VideoWidth, HisiChannel::VideoHeight, &mqtt, lanes);
        recogns.push_back(recogn);
        recogn->Start();
    }

    vector<DetectChannel*> detects;
    for (int i = 0; i < videoCount/DetectChannel::ItemCount; ++i)
    {
        vector<LaneDetector*> lanes;
        for (vector<Lane>::iterator it = channels[i].Lanes.begin(); it != channels[i].Lanes.end(); ++it)
        {
            lanes.push_back(new LaneDetector(StringEx::Combine(channels[i].ChannelUrl, "_", it->LaneId), *it));
        }
        DetectChannel* detect=new DetectChannel(i,HisiChannel::VideoWidth, HisiChannel::VideoHeight, &mqtt, lanes,recogns[i / RecognChannel::ItemCount]);
        detects.push_back(detect);
        detect->Start();
    }

    vector<HisiChannel*> decodes;
    for (int i = 0; i < videoCount; ++i)
    {
        HisiChannel* decode = new HisiChannel(i, detects[i/DetectChannel::ItemCount]);
        decodes.push_back(decode);
        //int r = decode->InitRtsp(channels[i].ChannelUrl);
        int r = decode->InitFile(channels[i].ChannelUrl, true, true);
        if (r == 0)
        {
            decode->Start();
        }
    }
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1));
    }

    SeemmoSDK::Uninit();
    HisiChannel::UninitHisi(videoCount);
    HisiChannel::UninitFFmpeg();
	return 0;
}