#pragma once
#include "mosquitto.h"

#include "Observable.h"
#include "LogPool.h"
#include "Thread.h"

namespace Saitama
{
    //mqtt接收消息事件参数
    class MqttReceivedEventArgs
    {
    public:

        /**
* @brief: 分页读取日志
* @param: logDIrectory 日志目录
* @param: logName 日志名称
* @param: logDate 日志日期
* @param: logLevel 日志级别，为0时查询所有
* @param: logEvent 日志事件，为0时查询所有
* @param: pageNum 分页页码
* @param: pageSize 分页数量
* @param: hasTotal 是否查询总数
* @return: 第一个参数表示日志查询结果集合，第二个参数表示总数，如果hasTotal为false则返回0
*/
        MqttReceivedEventArgs()
            :Message()
        {

        }

        /**
        * @brief: 构造函数
        * @param: message mqtt接收到的消息
        */
        MqttReceivedEventArgs(const char* message)
            :Message(message)
        {

        }

        //mqtt接收到的消息
        std::string Message;
    };

    //mqtt客户端线程
    class MqttChannel:public ThreadObject
    {
    public:

        /**
        * @brief: 构造函数
        * @param: ip mqtt服务端地址
        * @param: port mqtt服务端端口，默认为1883
        */
        MqttChannel(const std::string& ip, int port=1883);

        /**
        * @brief: mqtt连接回调函数
        * @param: mosq mosq实例
        * @param: userdata 自定义数据，此时表示this指针
        * @param: result 连接结果
        */
        static void ConnectedEventHandler(struct mosquitto* mosq, void* userdata, int result);

        /**
        * @brief: mqtt接收消息回调函数
        * @param: mosq mosq实例
        * @param: userdata 自定义数据，此时表示this指针
        * @param: message 接收到的消息
        */
        static void ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);

        //mqtt接收消息事件
        Observable<MqttReceivedEventArgs> MqttReceived;
    protected:

        void StartCore();

    private:

        //检测mqtt服务端网络的时间间隔(秒)
        static const int KeepAlive;
        //连接mqtt服务端的时间间隔(毫秒)
        static const int ConnectSpan;

        //mqtt服务端地址
        std::string _ip;
        //mqtt服务端端口
        int _port;


    };
}


