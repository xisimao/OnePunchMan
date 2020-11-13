#pragma once
#include "EventData.h"
#include "EventDetector.h"
#include "EncodeChannel.h"
#include "TrafficStartup.h"
#include "EventDataChannel.h"

namespace OnePunchMan
{
    //事件系统启动线程
    class EventStartup : public TrafficStartup
    {
    public:
        /**
        * 构造函数
        */
        EventStartup();

        /**
        * 析构函数
        */
        ~EventStartup();

        void Update(HttpReceivedEventArgs* e);

    protected:
        void UpdateDb();

        void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<FrameHandler*>* handlers, std::vector<TrafficDetector*>* detectors);

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        //通道检测集合,等于视频总数
        std::vector<EventDetector*> _detectors;

        EncodeChannel _encode;
        EventDataChannel* _data;
    };
}

