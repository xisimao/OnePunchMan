#pragma once

#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "VideoChannel.h"

namespace Saitama
{
    //数据分发和收集线程
    class DataChannel :public ThreadObject, public IObserver<MqttReceivedEventArgs>
    {
    public:

        /**
        * @brief: 构造函数
        * @param: ip mqtt服务端地址
        * @param: port mqtt服务端端口，默认为1883
        */
        DataChannel(const std::string& ip, int port);

        /**
        * @brief: mqtt消息接收事件函数
        * @param: e mqtt消息接收事件参数
        */
        void Update(MqttReceivedEventArgs* e);

    protected:

        void StartCore();

    private:

        static const std::string DetectTopic;
        static const std::string ChannelTopic;
        static const std::string ChannelRequestTopic;
        static const std::string TrafficTopic;
        static const std::string IOTopic;

        //mqtt服务端地址
        std::string _ip;
        //mqtt服务端端口
        int _port;

        //视频通道线程集合
        std::vector<VideoChannel*> _videoChannels;

        //上一次收集数据的分钟
        int _lastMinute;
    };
}

