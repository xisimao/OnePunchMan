#pragma once

#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "Channel.h"

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

        /**
        * @brief: 从json数据中获取检测项集合
        * @param: json json数据
        * @param: key 检测项字段的键
        * @return: 检测项集合
        */
        std::vector<DetectItem> GetDetectItems(const std::string& json, const std::string& key, long long timeStamp);

        /**
        * @brief: 处理检测数据
        * @param: json json数据
        */
        void HandleDetect(const std::string& json);

        /**
        * @brief: 处理通道数据
        * @param: json json数据
        */
        void HandleChannel(const std::string& json);

        /**
        * @brief: 收集流量数据
        * @param: now 当前时间
        */
        void CollectFlow(const DateTime& now);

        //检测数据 sdk->程序
        static const std::string DetectTopic;
        //通道 web->程序
        static const std::string ChannelTopic;
        //请求通道 程序->web
        static const std::string ChannelRequestTopic;
        //流量 程序->web
        static const std::string TrafficTopic;
        //IO状态 程序->web
        static const std::string IOTopic;

        //mqtt客户端
        MqttChannel* _mqtt;

        //mqtt服务端地址
        std::string _ip;
        //mqtt服务端端口
        int _port;

        //视频通道线程集合
        std::vector<Channel*> _channels;

        //上一次收集数据的分钟
        int _lastMinute;
    };
}

