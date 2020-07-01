#pragma once
#include "EventData.h"
#include "EventDetector.h"
#include "HisiEncodeChannel.h"
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
        void UpdateDb();

        void InitThreads(MqttChannel* mqtt, std::vector<HisiDecodeChannel*>* decodes, std::vector<TrafficDetector*>* detectors, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns);

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        /**
        * @brief: 校验流量通道
        * @param: 流量通道
        * @return: 检查成功返回空字符串，否则返回错误原因
        */
        std::string CheckChannel(EventChannel* channel);

        //通道检测集合，等于视频总数
        std::vector<EventDetector*> _detectors;
        //编码
        HisiEncodeChannel _encode;
    };
}

