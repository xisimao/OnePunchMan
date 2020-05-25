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
        * @brief: 析构函数
        */
        ~FlowStartup();

    protected:

        void InitDetectors(MqttChannel* mqtt, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns);

        void InitDecodes();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        //通道检测集合，等于视频总数
        std::vector<FlowDetector*> _detectors;

    };
}

