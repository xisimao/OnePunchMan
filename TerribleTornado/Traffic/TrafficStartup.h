#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "HttpHandler.h"
#include "Command.h"
#include "MqttChannel.h"
#include "DecodeChannel.h"
#include "TrafficDetector.h"
#include "SocketMaid.h"
#include "TrafficData.h"
#include "SqliteLogger.h"
#include "DG_FrameHandler.h"

namespace OnePunchMan
{
    //流量系统启动线程
    class TrafficStartup 
        : public ThreadObject
        , public IObserver<HttpReceivedEventArgs>
        , public IObserver<MqttDisconnectedEventArgs>
    {
    public:

        /**
        * 构造函数
        */
        TrafficStartup();

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

   
    protected:
        void StartCore();

        /**
        * 初始化线程集合
        * @param mqtt mqtt
        * @param decodes 解码类集合
        * @param handlers 视频帧处理类
        * @param detectors 交通检测类集合
        */
        virtual void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<FrameHandler*>* handlers, std::vector<TrafficDetector*>* detectors) = 0;

        /**
        * 初始化通道集合
        */
        virtual void InitChannels() = 0;

        /**
        * 升级数据库
        */
        virtual void UpdateDb() = 0;

        /**
        * 获取通道json数据
        * @param host 请求地址
        * @param channelIndex 通道序号
        * @return 通道json数据
        */
        virtual std::string GetChannelJson(const std::string& host, int channelIndex)=0;

        /**
        * 设置通道集合
        * @param e http消息接收事件参数
        */
        virtual void SetDevice(HttpReceivedEventArgs* e)=0;

        /**
        * 设置通道
        * @param e http消息接收事件参数
        */
        virtual void SetChannel(HttpReceivedEventArgs* e)=0;

        /**
        * 删除通道
        * @param channelIndex 通道序号
        * @return 删除成功返回true,否则返回false
        */
        virtual bool DeleteChannel(int channelIndex)=0;

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

        //解码线程集合,等于视频总数
        std::vector<DecodeChannel*> _decodes;

    private:
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
        //帧处理集合
        std::vector<FrameHandler*> _handlers;
        //交通检测集合
        std::vector<TrafficDetector*> _detectors;
    };
}

