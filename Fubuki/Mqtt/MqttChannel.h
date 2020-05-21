#pragma once
#include "mosquitto.h"

#include "Observable.h"
#include "LogPool.h"
#include "Thread.h"

namespace OnePunchMan
{
    //mqtt状态
    enum class MqttStatus
    {
        //初始化
        None,
        //连接上
        Connected,
        //断开连接
        Disconnected
    };

    //mqtt接收消息事件参数
    class MqttReceivedEventArgs
    {
    public:

        /**
        * @brief: 构造函数
        */
        MqttReceivedEventArgs()
            :Topic(), Message()
        {

        }

        /**
        * @brief: 构造函数
        * @param: message mqtt接收到的消息
        */
        MqttReceivedEventArgs(const char* topic, const char* message)
            :Topic(topic), Message(message)
        {

        }

        //mqtt接收到的主题
        std::string Topic;
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
        MqttChannel(const std::string& ip, int port);

        /**
        * @brief: 构造函数
        * @param: ip mqtt服务端地址
        * @param: port mqtt服务端端口，默认为1883
        * @param: topics 订阅的主题集合
        */
        MqttChannel(const std::string& ip, int port, std::vector<std::string> topics);

        /**
        * @brief: mqtt接收消息回调函数
        * @param: mosq mosq实例
        * @param: userdata 自定义数据，此时表示this指针
        * @param: message 接收到的消息
        */
        static void ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);

        //mqtt接收消息事件
        Observable<MqttReceivedEventArgs> MqttReceived;

        /**
        * @brief: 初始化mqtt sdk
        */
        static void Init();

        /**
        * @brief: 卸载mqtt sdk
        */
        static void Uninit();

        /**
         * @brief: 获取mqtt是否连接成功
         * @return: mqtt是否连接成功
         */
        MqttStatus Status();

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
        static const int SleepTime;

        //mosq实例
        struct mosquitto* _mosq;

        //mqtt服务端地址
        std::string _ip;
        //mqtt服务端端口
        int _port;
        //订阅的主题集合
        std::vector<std::string> _topics;
        //连接状态
        MqttStatus _status;
    };
}


