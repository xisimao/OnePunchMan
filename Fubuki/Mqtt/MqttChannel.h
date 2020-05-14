#pragma once
#include "mosquitto.h"

#include "LogPool.h"
#include "Thread.h"

namespace OnePunchMan
{
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
        MqttChannel(const std::string& ip, int port);

        /**
         * @brief: ��ȡmqtt�Ƿ����ӳɹ�
         * @return: mqtt�Ƿ����ӳɹ�
         */
        bool Connected();

        /**
         * @brief: mqtt������Ϣ
         * @param: topic ����
         * @param: message ���͵���Ϣ
         * @param: qos ��������
         * @return: ����true��ʾ���ͳɹ�
         */
        bool Send(const std::string& topic,const std::string& message, int qos = 0);

        /**
         * @brief: mqtt������Ϣ
         * @param: topic ����
         * @param: message ������Ϣ���ֽ���
         * @param: message ������Ϣ���ֽ�������
         * @param: qos ��������
         * @return: ����true��ʾ���ͳɹ�
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
        static const int PollTime;

        //ͬ����
        std::timed_mutex _mutex;
        //mosqʵ��
        struct mosquitto* _mosq;

        //mqtt����˵�ַ
        std::string _ip;
        //mqtt����˶˿�
        int _port;

        //����״̬
        bool _connected;
    };
}


