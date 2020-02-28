#pragma once

#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "Channel.h"

namespace Saitama
{
    //���ݷַ����ռ��߳�
    class DataChannel :public ThreadObject, public IObserver<MqttReceivedEventArgs>
    {
    public:

        /**
        * @brief: ���캯��
        * @param: ip mqtt����˵�ַ
        * @param: port mqtt����˶˿ڣ�Ĭ��Ϊ1883
        */
        DataChannel(const std::string& ip, int port);

        /**
        * @brief: mqtt��Ϣ�����¼�����
        * @param: e mqtt��Ϣ�����¼�����
        */
        void Update(MqttReceivedEventArgs* e);

    protected:

        void StartCore();

    private:

        /**
        * @brief: ��json�����л�ȡ������
        * @param: json json����
        * @param: key ������ֶεļ�
        * @return: ������
        */
        std::vector<DetectItem> GetDetectItems(const std::string& json, const std::string& key, long long timeStamp);

        /**
        * @brief: ����������
        * @param: json json����
        */
        void HandleDetect(const std::string& json);

        /**
        * @brief: ����ͨ������
        * @param: json json����
        */
        void HandleChannel(const std::string& json);

        /**
        * @brief: �ռ���������
        * @param: now ��ǰʱ��
        */
        void CollectFlow(const DateTime& now);

        //������� sdk->����
        static const std::string DetectTopic;
        //ͨ�� web->����
        static const std::string ChannelTopic;
        //����ͨ�� ����->web
        static const std::string ChannelRequestTopic;
        //���� ����->web
        static const std::string TrafficTopic;
        //IO״̬ ����->web
        static const std::string IOTopic;

        //mqtt�ͻ���
        MqttChannel* _mqtt;

        //mqtt����˵�ַ
        std::string _ip;
        //mqtt����˶˿�
        int _port;

        //��Ƶͨ���̼߳���
        std::vector<Channel*> _channels;

        //��һ���ռ����ݵķ���
        int _lastMinute;
    };
}

