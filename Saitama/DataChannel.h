#pragma once

#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "MqttChannel.h"
#include "VideoChannel.h"

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

        static const std::string DetectTopic;
        static const std::string ChannelTopic;
        static const std::string ChannelRequestTopic;
        static const std::string TrafficTopic;
        static const std::string IOTopic;

        //mqtt����˵�ַ
        std::string _ip;
        //mqtt����˶˿�
        int _port;

        //��Ƶͨ���̼߳���
        std::vector<VideoChannel*> _videoChannels;

        //��һ���ռ����ݵķ���
        int _lastMinute;
    };
}

