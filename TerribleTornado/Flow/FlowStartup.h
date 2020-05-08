#pragma once
#include "FlowData.h"
#include "FlowDetector.h"
#include "TrafficStartup.h"

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class FlowStartup : public TrafficStartup
    {
    public:

        /**
        * @brief: ���캯��
        */
        FlowStartup();

        /**
        * @brief: ��������
        */
        ~FlowStartup();

    protected:
        void PollCore();

        std::vector<TrafficDetector*> InitDetectors();

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        //����mqtt����
        static const std::string FlowTopic;

        //ͨ����⼯�ϣ�������Ƶ����
        std::vector<FlowDetector*> _detectors;

        //��һ���ռ����ݵķ���
        int _lastMinute;

    };
}

