#pragma once
#include "EventData.h"
#include "EventDetector.h"
#include "TrafficStartup.h"

namespace OnePunchMan
{
    //�¼�ϵͳ�����߳�
    class EventStartup : public TrafficStartup
    {
    public:
        /**
        * @brief: ���캯��
        */
        EventStartup();

        /**
        * @brief: ��������
        */
        ~EventStartup();

    protected:
        void InitSoftVersion();

        void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<TrafficDetector*>* detectors, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns);

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        //ͨ����⼯�ϣ�������Ƶ����
        std::vector<EventDetector*> _detectors;
    };
}

