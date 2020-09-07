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
#include "TrafficData.h"
#ifndef _WIN32
#include "clientsdk.h"
#endif // !_WIN32

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
        * @brief: 构造函数
        */
        TrafficStartup();

        //通道总数
        static const int ChannelCount;
        //检测线程总数
        static const int DetectCount;
        //识别线程总数
        static const int RecognCount;

        /**
        * @brief: http消息接收事件函数
        * @param: e http消息接收事件参数
        */
        virtual void Update(HttpReceivedEventArgs* e);

        /**
        * @brief: mqtt断开事件函数
        * @param: e mqtt断开事件参数
        */
        void Update(MqttDisconnectedEventArgs* e);

    protected:
        void StartCore();

        /**
        * @brief: 初始化线程集合
        * @param: mqtt mqtt
        * @param: decodes 解码类集合
        * @param: detectors 交通检测类集合
        * @param: detects 视频检测类集合
        * @param: recogns 视频识别类集合
        * @param: loginHandler 登陆句柄
        */
        virtual void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<TrafficDetector*>* detectors, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns,int loginHandler) = 0;

        /**
        * @brief: 初始化通道集合
        */
        virtual void InitChannels() = 0;

        /**
        * @brief: 升级数据库
        */
        virtual void UpdateDb() = 0;

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
        * @brief: 获取通道的检测报告
        * @param: channelIndex 通道序号
        * @param: e http消息接收事件参数
        */
        virtual void GetReport(int channelIndex,HttpReceivedEventArgs* e){}

        /**
        * @brief: 判断通道序号是否可用
        * @param: channelIndex 通道序号
        * @return: 返回ture表示通道序号可用，否则返回false
        */
        bool ChannelIndexEnable(int channelIndex);

        /**
        * @brief: 检查通道数据项
        * @param: channel 通道数据
        * @return: 返回空字符串表示检查通过，否则返回错误原因
        */
        std::string CheckChannel(TrafficChannel* channel);

        /**
        * @brief: 获取错误信息json数据
        * @param: field 字段名
        * @param: message 错误信息
        * @return: json数据
        */
        std::string GetErrorJson(const std::string& field, const std::string& message);
        
        /**
        * @brief: 根据通道填充Json字符串
        * @param: channelJson 要填充的json字符串
        * @param: channel 通道数据
        * @param: host 本机地址
        */
        void FillChannelJson(std::string* channelJson, const TrafficChannel* channel, const std::string& host);
        
        /**
        * @brief: 根据通道json填充通道数据
        * @param: channel 要填充的通道数据
        * @param: jd json字符串
        */
        void FillChannel(TrafficChannel* channel, const JsonDeserialization& jd);
        
        /**
        * @brief: 根据通道数组json填充通道数据
        * @param: channel 要填充的通道数据
        * @param: jd json字符串
        * @param: itemIndex 通道所在数组的序号
        */
        void FillChannel(TrafficChannel* channel, const JsonDeserialization& jd, int itemIndex);

        /**
        * @brief: 获取url是否是指定的前缀
        * @param: url url
        * @param: key 前缀
        * @return: 返回true表示url有指定前缀
        */
        bool UrlStartWith(const std::string& url, const std::string& key);

        //解码线程集合，等于视频总数
        std::vector<DecodeChannel*> _decodes;

    private:
        /**
        * @brief: 查询设备
        * @param: e http消息接收事件参数
        */
        void GetDevice(HttpReceivedEventArgs* e);

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

        //交通检测类集合，等于视频总数
        std::vector<TrafficDetector*> _detectors;
        //视频检测线程集合，等于视频总数
        std::vector<DetectChannel*> _detects;
        //视频识别线程集合，等于视频总数/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;
    };
}

