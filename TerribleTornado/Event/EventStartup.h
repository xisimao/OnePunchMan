#pragma once
#include "EventData.h"
#include "EventDetector.h"
#include "EncodeChannel.h"
#include "TrafficStartup.h"
#include "EventDataChannel.h"

namespace OnePunchMan
{
    //�¼�ϵͳ�����߳�
    class EventStartup : public TrafficStartup
    {
    public:
        /**
        * ���캯��
        */
        EventStartup();

        /**
        * ��������
        */
        ~EventStartup();

        void Update(HttpReceivedEventArgs* e);

    protected:
        void UpdateDb();

        void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<FrameHandler*>* handlers, std::vector<TrafficDetector*>* detectors);

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        //ͨ����⼯��,������Ƶ����
        std::vector<EventDetector*> _detectors;

        EncodeChannel _encode;
        EventDataChannel* _data;
    };
}

