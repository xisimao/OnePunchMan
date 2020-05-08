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
        * @brief: 析构函数
        */
        ~EventStartup();

    protected:
        std::vector<TrafficDetector*> InitDetectors();

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

