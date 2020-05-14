#pragma once
#include "mosquitto.h"

#include "LogPool.h"
#include "Thread.h"

namespace OnePunchMan
{
    //mqtt客户端线程
    class MqttChannel:public ThreadObject
    {
    public:

        /**
        * @brief: 构造函数
        * @param: ip mqtt服务端地址
        * @param: port mqtt服务端端口，默认为1883
        * @param: topics 订阅的主题集合
        */
        MqttChannel(const std::string& ip, int port);

        /**
         * @brief: 获取mqtt是否连接成功
         * @return: mqtt是否连接成功
         */
        bool Connected();

        /**
         * @brief: mqtt发送消息
         * @param: topic 主题
         * @param: message 发送的消息
         * @param: qos 服务质量
         * @return: 返回true表示发送成功
         */
        bool Send(const std::string& topic,const std::string& message, int qos = 0);

        /**
         * @brief: mqtt发送消息
         * @param: topic 主题
         * @param: message 发送消息的字节流
         * @param: message 发送消息的字节流长度
         * @param: qos 服务质量
         * @return: 返回true表示发送成功
         */
        bool Send(const std::string& topic, const unsigned char* message,unsigned int size,int qos =0);

    protected:

        void StartCore();

    private:

        //检测mqtt服务端网络的时间间隔(秒)
        static const int KeepAlive;
        //连接mqtt服务端的时间间隔(毫秒)
        static const int ConnectSpan;
        //线程休眠时间(毫秒)
        static const int PollTime;

        //同步锁
        std::timed_mutex _mutex;
        //mosq实例
        struct mosquitto* _mosq;

        //mqtt服务端地址
        std::string _ip;
        //mqtt服务端端口
        int _port;

        //连接状态
        bool _connected;
    };
}


