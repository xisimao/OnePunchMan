#pragma once
#include "FlowData.h"
#include "FlowDetector.h"
#include "TrafficStartup.h"

namespace OnePunchMan
{
    //流量系统启动线程
    class FlowStartup : public TrafficStartup
    {
    public:

        /**
        * @brief: 构造函数
        */
        FlowStartup();

        /**
        * @brief: 析构函数
        */
        ~FlowStartup();

    protected:
        void PollCore();

        std::vector<TrafficDetector*> InitDetectors();

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        //流量mqtt主题
        static const std::string FlowTopic;

        //通道检测集合，等于视频总数
        std::vector<FlowDetector*> _detectors;

        //上一次收集数据的分钟
        int _lastMinute;

    };
}

