#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "HttpHandler.h"
#include "Command.h"
#include "DecodeChannel.h"
#include "FlowDetector.h"
#include "EventDetector.h"
#include "SocketMaid.h"
#include "TrafficData.h"
#ifndef _WIN32
#include "clientsdk.h"
#endif // !_WIN32

namespace OnePunchMan
{
    //流量系统启动线程
    class TrafficStartup
        : public IObserver<HttpReceivedEventArgs>
    {
    public:
        /**
        * 构造函数
        */
        TrafficStartup();

        /**
        * 析构函数
        */
        virtual ~TrafficStartup();

        //通道总数
        static const int ChannelCount;

        /**
        * http消息接收事件函数
        * @param e http消息接收事件参数
        */
        virtual void Update(HttpReceivedEventArgs* e);

        /**
        * 启动系统
        */
        virtual void Start() = 0;

    protected:
        /**
        * 供子类实现的更新通道
        * @param channel 通道
        */
        virtual void UpdateChannel(const TrafficChannel& channel);

        /**
        * 供子类实现的截图
        * @param channelIndex 通道序号
        */
        virtual void Screenshot(int channelIndex)=0;

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

    private:
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
        * 清空通道
        * @param channelIndex 通道序号
        */
        void ClearChannel(int channelIndex);

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
        bool CheckChannelIndex(int channelIndex);

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
        * 获取通道json数据
        * @param channel 通道数据
        * @param host 本机地址
        */
        std::string GetChannelJson(const std::string& host,int channelIndex);

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
        * 查询设备
        * @param e http消息接收事件参数
        */
        void GetDevice(HttpReceivedEventArgs* e);
    };
}

