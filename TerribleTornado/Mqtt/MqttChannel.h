#pragma once
#include "mosquitto.h"

#include "Observable.h"
#include "LogPool.h"
#include "Thread.h"

namespace Saitama
{
    //mqtt������Ϣ�¼�����
    class MqttReceivedEventArgs
    {
    public:

        /**
        * @brief: ���캯��
        */
        MqttReceivedEventArgs()
            :Topic(),Message()
        {

        }

        /**
        * @brief: ���캯��
        * @param: message mqtt���յ�����Ϣ
        */
        MqttReceivedEventArgs(const char* topic,const char* message)
            :Topic(topic),Message(message)
        {

        }

        //mqtt���յ�������
        std::string Topic;
        //mqtt���յ�����Ϣ
        std::string Message;
    };

    //mqtt�ͻ����߳�
    class MqttChannel:public ThreadObject
    {
    public:

        /**
        * @brief: ���캯��
        * @param: ip mqtt����˵�ַ
        * @param: port mqtt����˶˿ڣ�Ĭ��Ϊ1883
        * @param: topics ���ĵ����⼯��
        */
        MqttChannel(const std::string& ip, int port,std::vector<std::string> topics);

        /**
        * @brief: mqtt���ӻص�����
        * @param: mosq mosqʵ��
        * @param: userdata �Զ������ݣ���ʱ��ʾthisָ��
        * @param: result ���ӽ��
        */
        static void ConnectedEventHandler(struct mosquitto* mosq, void* userdata, int result);

        /**
        * @brief: mqtt������Ϣ�ص�����
        * @param: mosq mosqʵ��
        * @param: userdata �Զ������ݣ���ʱ��ʾthisָ��
        * @param: message ���յ�����Ϣ
        */
        static void ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);

        //mqtt������Ϣ�¼�
        Observable<MqttReceivedEventArgs> MqttReceived;

        /**
         * @brief: mqtt������Ϣ
         * @param: topic ����
         * @param: message ���͵���Ϣ
         * @param: lock �Ƿ��ͬ����
         * @return: ����true��ʾ���ͳɹ�
         */
        bool Send(const std::string& topic,const std::string& message,bool lock =true);

    protected:

        void StartCore();

    private:

        //���mqtt����������ʱ����(��)
        static const int KeepAlive;
        //����mqtt����˵�ʱ����(����)
        static const int ConnectSpan;
        //�߳�����ʱ��(����)
        static const int PollTime;

        //ͬ����
        std::mutex _mutex;
        //mosqʵ��
        struct mosquitto* _mosq;

        //mqtt����˵�ַ
        std::string _ip;
        //mqtt����˶˿�
        int _port;
        //���ĵ����⼯��
        std::vector<std::string> _topics;

        //����״̬
        int _status = -1;
    };
}


