#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "HttpHandler.h"
#include "Command.h"
#include "SeemmoSDK.h"
#include "FlowData.h"
#include "MqttChannel.h"
#include "DetectChannel.h"
#include "DecodeChannel.h"
#include "FlowChannelDetector.h"
#include "SocketMaid.h"

namespace OnePunchMan
{
    //流量系统启动线程
    class FlowStartup :public ThreadObject
        , public IObserver<HttpReceivedEventArgs>
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

        /**
        * @brief: 初始化系统
        */
        bool Init();

        /**
        * @brief: http消息接收事件函数
        * @param: e http消息接收事件参数
        */
        void Update(HttpReceivedEventArgs* e);

    protected:

        void StartCore();

    private:

        /**
        * @brief: 查询设备
        * @param: e http消息接收事件参数
        */
        void GetDevice(HttpReceivedEventArgs* e);
        
        /**
        * @brief: 设置通道集合
        * @param: e http消息接收事件参数
        */
        void SetDevice(HttpReceivedEventArgs* e);
        
        /**
        * @brief: 获取通道
        * @param: e http消息接收事件参数
        */
        void GetChannel(HttpReceivedEventArgs* e);
  
        /**
        * @brief: 设置通道
        * @param: e http消息接收事件参数
        */
        void SetChannel(HttpReceivedEventArgs* e);
        
        /**
        * @brief: 删除通道
        * @param: e http消息接收事件参数
        */
        void DeleteChannel(HttpReceivedEventArgs* e);
        
        /**
        * @brief: 设置通道
        * @param: channel 通道
        */
        void SetChannel(const FlowChannel& channel);
        
        /**
        * @brief: 删除通道
        * @param: channelIndex 通道序号
        */
        void DeleteChannel(int channelIndex);

        /**
        * @brief: 判断通道序号是否可用
        * @param: channelIndex 通道序号
        * @return: 返回ture表示通道序号可用，否则返回false
        */
        bool ChannelIndexEnable(int channelIndex);

        /**
        * @brief: 检查通道信息
        * @param: channel 通道
        * @return: 检查成功返回空字符串，否则返回错误原因
        */
        std::string CheckChannel(const FlowChannel& channel);

        /**
        * @brief: 获取通道json数据
        * @param: e http消息接收事件参数
        * @param: channel 通道
        * @return: 通道json数据
        */
        std::string GetChannelJson(HttpReceivedEventArgs* e,const FlowChannel& channel);

        /**
        * @brief: 获取url是否是指定的前缀
        * @param: url url
        * @param: key 前缀
        * @return: 返回true表示url有指定前缀
        */
        bool UrlStartWith(const std::string& url, const std::string& key);
       
        /**
        * @brief: 获取url中的id
        * @param: url url
        * @param: key 前缀
        * @return: id
        */
        std::string GetId(const std::string& url, const std::string& key);
       
        /**
        * @brief: 获取错误信息json数据
        * @param: field 字段名
        * @param: message 错误信息
        * @return: json数据
        */
        std::string GetErrorJson(const std::string& field, const std::string& message);

        //流量mqtt主题
        static const std::string FlowTopic;

        //算法是否初始化成功
        bool _sdkInited;

        //socket连接
        SocketMaid* _socketMaid;
        //http消息解析
        HttpHandler _handler;
        //mqtt
        MqttChannel* _mqtt;
        //检测线程集合，等于视频总数
        std::vector<DetectChannel*> _detects;
        //识别线程集合，等于视频总数/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;
        //通道检测集合，等于视频总数
        std::vector<FlowChannelDetector*> _detectors;
        //解码线程同步锁
        std::mutex _decodeMutex;
        //解码线程集合，等于视频总数，如果视频清空则为NULL
        std::vector<DecodeChannel*> _decodes;

        //系统启动时间
        DateTime _startTime;
        //上一次收集数据的分钟
        int _lastMinute;
    };
}

