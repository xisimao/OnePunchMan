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
#include "FlowDetector.h"
#include "EventDetector.h"
#include "SocketMaid.h"
#include "TrafficData.h"
#include "SqliteLogger.h"
#ifndef _WIN32
#include "clientsdk.h"
#endif // !_WIN32

namespace OnePunchMan
{
    //流量系统启动线程
    class TrafficStartup 
        : public IObserver<HttpReceivedEventArgs>
        , public IObserver<MqttDisconnectedEventArgs>
    {
    public:

        /**
        * 构造函数
        */
        TrafficStartup();

        /**
        * 析构函数
        */
        ~TrafficStartup();

        //通道总数
        static const int ChannelCount;
        //检测线程总数
        static const int DetectCount;
        //识别线程总数
        static const int RecognCount;

        /**
        * http消息接收事件函数
        * @param e http消息接收事件参数
        */
        virtual void Update(HttpReceivedEventArgs* e);

        /**
        * mqtt断开事件函数
        * @param e mqtt断开事件参数
        */
        void Update(MqttDisconnectedEventArgs* e);

        /**
        * 启动系统
        */
        void Start();
    private:
        /**
        * 获取通道json数据
        * @param host 请求地址
        * @param channelIndex 通道序号
        * @return 通道json数据
        */
        std::string GetChannelJson(const std::string& host, int channelIndex);

        /**
        * 设置通道集合
        * @param e http消息接收事件参数
        */
        void SetDevice(HttpReceivedEventArgs* e);

        /**
        * 设置通道
        * @param e http消息接收事件参数
        */
        void SetChannel(HttpReceivedEventArgs* e);

        /**
        * 删除通道
        * @param channelIndex 通道序号
        * @return 删除成功返回true,否则返回false
        */
        bool DeleteChannel(int channelIndex);

        /**
        * 检查通道数据项
        * @param channel 通道数据
        * @return 返回空字符串表示检查通过,否则返回错误原因
        */
        std::string CheckChannel(TrafficChannel* channel);

        /**
        * 判断通道序号是否可用
        * @param channelIndex 通道序号
        * @return 返回ture表示通道序号可用,否则返回false
        */
        bool ChannelIndexEnable(int channelIndex);

        /**
        * 根据通道填充Json字符串
        * @param channelJson 要填充的json字符串
        * @param channel 通道数据
        * @param host 本机地址
        */
        void FillChannelJson(std::string* channelJson, const TrafficChannel* channel, const std::string& host);
        
        /**
        * 根据通道json填充通道数据
        * @param channel 要填充的通道数据
        * @param jd json字符串
        */
        void FillChannel(TrafficChannel* channel, const JsonDeserialization& jd);
        
        /**
        * 根据通道数组json填充通道数据
        * @param channel 要填充的通道数据
        * @param jd json字符串
        * @param itemIndex 通道所在数组的序号
        */
        void FillChannel(TrafficChannel* channel, const JsonDeserialization& jd, int itemIndex);

        /**
        * 获取url是否是指定的前缀
        * @param url url
        * @param key 前缀
        * @return 返回true表示url有指定前缀
        */
        bool UrlStartWith(const std::string& url, const std::string& key);

        /**
        * 获取url中的id
        * @param url url
        * @param key 前缀
        * @return id
        */
        std::string GetId(const std::string& url, const std::string& key);

        /**
         * 初始化线程集合
         * @param loginHandler 登陆句柄
         */
        void InitThreads(int loginHandler);

        /**
        * 初始化通道集合
        */
        void InitChannels();

        /**
        * 查询设备
        * @param e http消息接收事件参数
        */
        void GetDevice(HttpReceivedEventArgs* e);

        //系统启动时间
        DateTime _startTime;
        //算法是否初始化成功
        bool _sdkInited;
        //软件版本
        std::string _softwareVersion;
        //算法版本
        std::string _sdkVersion;
        //socket连接
        SocketMaid* _socketMaid;
        //http消息解析
        HttpHandler _handler;
        //mqtt
        MqttChannel* _mqtt;
        //事件数据入库线程
        DataChannel* _data;
        //编码
        EncodeChannel* _encode;
        //解码线程集合,等于视频总数
        std::vector<DecodeChannel*> _decodes;
        //流量检测类集合,等于视频总数
        std::vector<FlowDetector*> _flowDetectors;
        //事件检测类集合,等于视频总数
        std::vector<EventDetector*> _eventDetectors;
        //视频检测线程集合,等于视频总数
        std::map<int,DetectChannel*> _detects;
        //视频识别线程集合,等于视频总数/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;


    };
}

