#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "HttpHandler.h"
#include "Command.h"
#include "SeemmoSDK.h"
#include "MqttChannel.h"
#include "DetectChannel.h"
#include "DecodeChannel.h"
#include "TrafficDetector.h"
#include "SocketMaid.h"

namespace OnePunchMan
{
    //流量系统启动线程
    class TrafficStartup :public ThreadObject
        , public IObserver<HttpReceivedEventArgs>
    {
    public:

        /**
        * @brief: 构造函数
        */
        TrafficStartup();

        /**
        * @brief: 析构函数
        */
        virtual ~TrafficStartup();

        /**
        * @brief: 初始化系统
        */
        bool Init();

        /**
        * @brief: http消息接收事件函数
        * @param: e http消息接收事件参数
        */
        void Update(HttpReceivedEventArgs* e);

        //通道总数
        static const int ChannelCount;

    protected:

        void StartCore();

        /**
        * @brief: 供子类实现的线程轮询方法
        */
        virtual void PollCore() {};

        /**
        * @brief: 初始化检测逻辑类集合
        * @return: 检测类集合
        */
        virtual std::vector<TrafficDetector*> InitDetectors() = 0;

        /**
        * @brief: 初始化通道集合
        */
        virtual void InitChannels() = 0;

        /**
        * @brief: 获取通道json数据
        * @param: host 请求地址
        * @param: channelIndex 通道序号
        * @return: 通道json数据
        */
        virtual std::string GetChannelJson(const std::string& host, int channelIndex)=0;

        /**
        * @brief: 设置通道集合
        * @param: e http消息接收事件参数
        */
        virtual void SetDevice(HttpReceivedEventArgs* e)=0;

        /**
        * @brief: 设置通道
        * @param: e http消息接收事件参数
        */
        virtual void SetChannel(HttpReceivedEventArgs* e)=0;

        /**
        * @brief: 删除通道
        * @param: channelIndex 通道序号
        * @return: 删除成功返回true，否则返回false
        */
        virtual bool DeleteChannel(int channelIndex)=0;

        /**
        * @brief: 设置解码器
        * @param: channelIndex 通道序号
        * @param: inputUrl 视频源地址
        * @param: outputUrl 视频输入地址
        */
        void SetDecode(int channelIndex, const std::string& inputUrl, const std::string& outputUrl);
        
        /**
        * @brief: 删除解码器
        * @param: channelIndex 通道序号
        */
        void DeleteDecode(int channelIndex);

        /**
        * @brief: 检查通道序号信息
        * @param: channelIndex 通道序号
        * @return: 检查成功返回空字符串，否则返回错误原因
        */
        std::string CheckChannel(int channelIndex);

        /**
        * @brief: 判断通道序号是否可用
        * @param: channelIndex 通道序号
        * @return: 返回ture表示通道序号可用，否则返回false
        */
        bool ChannelIndexEnable(int channelIndex);

        /**
        * @brief: 获取错误信息json数据
        * @param: field 字段名
        * @param: message 错误信息
        * @return: json数据
        */
        std::string GetErrorJson(const std::string& field, const std::string& message);

        //mqtt
        MqttChannel* _mqtt;

    private:
        /**
        * @brief: 查询设备
        * @param: e http消息接收事件参数
        */
        void GetDevice(HttpReceivedEventArgs* e);

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
        
        //系统启动时间
        DateTime _startTime;
        //算法是否初始化成功
        bool _sdkInited;

        //socket连接
        SocketMaid* _socketMaid;
        //http消息解析
        HttpHandler _handler;
        //检测线程集合，等于视频总数
        std::vector<DetectChannel*> _detects;
        //识别线程集合，等于视频总数/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;
        //解码线程同步锁
        std::mutex _decodeMutex;
        //解码线程集合，等于视频总数，如果视频清空则为NULL
        std::vector<DecodeChannel*> _decodes;

    };
}

