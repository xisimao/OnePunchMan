#pragma once
#include "EventData.h"
#include "EventDetector.h"
#include "TrafficStartup.h"

namespace OnePunchMan
{
    //事件系统启动线程
    class EventStartup : public TrafficStartup
    {
    public:
        /**
        * @brief: 构造函数
        */
        EventStartup();

        /**
        * @brief: 析构函数
        */
        ~EventStartup();

    protected:
        void InitSoftVersion();

        void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<TrafficDetector*>* detectors, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns);

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        //通道检测集合，等于视频总数
        std::vector<EventDetector*> _detectors;
    };
}

