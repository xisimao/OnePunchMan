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
        * @brief: ��������
        */
        ~EventStartup();

    protected:
        std::vector<TrafficDetector*> InitDetectors();

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

