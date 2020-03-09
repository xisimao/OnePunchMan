#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "VideoDetector.h"
#include "FlowChannelData.h"
#include "HttpHandler.h"

namespace Saitama
{


    //数据分发和收集线程
    class DataChannel :public ThreadObject
        , public IObserver<MqttReceivedEventArgs>
        , public IObserver<HttpReceivedEventArgs>
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

        /**
        * @brief: http消息接收事件函数
        * @param: e http消息接收事件参数
        */
        void Update(HttpReceivedEventArgs* e);

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
        * @brief: 从json数据中获取检测项
        * @param: json json数据
        * @return: 检测项
        */
        DetectItem GetDetectItem(const std::string& json);

        /**
        * @brief: 处理检测数据
        * @param: json json数据
        */
        void HandleDetect(const std::string& json);

        /**
        * @brief: 处理识别数据
        * @param: json json数据
        */
        void HandleRecognize(const std::string& json);

        /**
        * @brief: 收集流量数据
        * @param: now 当前时间
        */
        void CollectFlow(const DateTime& now);

        /**
        * @brief: 获取url是否是指定的前缀
        * @param: url url
        * @param: key 前缀
        * @return: 返回true表示url有指定前缀
        */
        bool UrlStartWith(const std::string& url, const std::string& key);

        /**
        * @brief: 获取url中的编号
        * @param: url url
        * @param: key 前缀
        * @return: 获取成功返回编号否则返回空字符串
        */
        std::string GetId(const std::string& url, const std::string& key);

        /**
        * @brief: 更新通道信息
        * @param: channel 通道信息
        */
        void UpdateChannel(const FlowChannel& channel);

        //检测数据 sdk->程序
        static const std::string DetectTopic;
        //识别数据 sdk->程序
        static const std::string RecognizeTopic;
        //流量 程序->web
        static const std::string FlowTopic;
        //流量 程序->web
        static const std::string VideoTopic;
        //IO状态 程序->web
        static const std::string IOTopic;

        //mqtt客户端
        MqttChannel* _mqtt;

        //mqtt服务端地址
        std::string _ip;
        //mqtt服务端端口
        int _port;

        //视频通道线程集合
        std::vector<VideoDetector*> _channels;

        //上一次收集数据的分钟
        int _lastMinute;
    };
}

