#pragma once
#include "mosquitto.h"

#include "Observable.h"
#include "LogPool.h"
#include "Thread.h"

namespace OnePunchMan
{
    //mqtt״̬
    enum class MqttStatus
    {
        //��ʼ��
        None,
        //������
        Connected,
        //�Ͽ�����
        Disconnected
    };

    //mqtt������Ϣ�¼�����
    class MqttReceivedEventArgs
    {
    public:

        /**
        * ���캯��
        */
        MqttReceivedEventArgs()
            :Topic(), Message()
        {

        }

        /**
        * ���캯��
        * @param message mqtt���յ�����Ϣ
        */
        MqttReceivedEventArgs(const char* topic, const char* message)
            :Topic(topic), Message(message)
        {

        }

        //mqtt���յ�������
        std::string Topic;
        //mqtt���յ�����Ϣ
        std::string Message;
    };

    //mqtt�Ͽ��¼�����
    class MqttDisconnectedEventArgs
    {

    };

    //mqtt�ͻ����߳�
    class MqttChannel:public ThreadObject
    {
    public:

        /**
        * ���캯��
        * @param ip mqtt����˵�ַ
        * @param port mqtt����˶˿�,Ĭ��Ϊ1883
        */
        MqttChannel(const std::string& ip, int port);

        /**
        * ���캯��
        * @param ip mqtt����˵�ַ
        * @param port mqtt����˶˿�,Ĭ��Ϊ1883
        * @param topics ���ĵ����⼯��
        */
        MqttChannel(const std::string& ip, int port, std::vector<std::string> topics);

        /**
        * mqtt������Ϣ�ص�����
        * @param mosq mosqʵ��
        * @param userdata �Զ�������,��ʱ��ʾthisָ��
        * @param message ���յ�����Ϣ
        */
        static void ReceivedEventHandler(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);

        //mqtt������Ϣ�¼�
        Observable<MqttReceivedEventArgs> MqttReceived;

        //mqtt�Ͽ��¼�
        Observable<MqttDisconnectedEventArgs> MqttDisconnected;

        /**
        * ��ʼ��mqtt sdk
        */
        static void Init();

        /**
        * ж��mqtt sdk
        */
        static void Uninit();

        /**
         * ��ȡmqtt�Ƿ����ӳɹ�
         * @return mqtt�Ƿ����ӳɹ�
         */
        MqttStatus Status();

        /**
         * mqtt������Ϣ
         * @param topic ����
         * @param message ���͵���Ϣ
         * @param qos ��������
         * @return ����true��ʾ���ͳɹ�
         */
        bool Send(const std::string& topic,const std::string& message, int qos = 0);

        /**
         * mqtt������Ϣ
         * @param topic ����
         * @param message ������Ϣ���ֽ���
         * @param message ������Ϣ���ֽ�������
         * @param qos ��������
         * @return ����true��ʾ���ͳɹ�
         */
        bool Send(const std::string& topic, const unsigned char* message,unsigned int size,int qos =0);

    protected:

        void StartCore();

    private:

        //���mqtt����������ʱ����(��)
        static const int KeepAlive;
        //����mqtt����˵�ʱ����(����)
        static const int ConnectSpan;
        //�߳�����ʱ��(����)
        static const int SleepTime;

        //mosqʵ��
        struct mosquitto* _mosq;

        //mqtt����˵�ַ
        std::string _ip;
        //mqtt����˶˿�
        int _port;
        //���ĵ����⼯��
        std::vector<std::string> _topics;
        //����״̬
        MqttStatus _status;
    };
}


