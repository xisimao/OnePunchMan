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
        * 构造函数
        */
        FlowStartup();

        /**
        * 析构函数
        */
        ~FlowStartup();

        void Update(HttpReceivedEventArgs* e);

    protected:
        void UpdateDb();

        void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<TrafficDetector*>* detectors, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns,int loginHandler);

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        /**
        * 获取通道的检测报告
        * @param channelIndex 通道序号
        * @param e http消息接收事件参数
        */
        void GetReport(int channelIndex, HttpReceivedEventArgs* e);

        std::string CheckFlowChannel(FlowChannel* channel);

        //通道检测集合,等于视频总数
        std::vector<FlowDetector*> _detectors;
        //数据合并
        DataMergeMap* _merge;

    };
}

