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
        void UpdateDb();

        void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes,EncodeChannel* encode, std::vector<TrafficDetector*>* detectors, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns,int loginHandler);

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

        void GetReport(int channelIndex,HttpReceivedEventArgs* e);

    private:
        //ͨ����⼯�ϣ�������Ƶ����
        std::vector<FlowDetector*> _detectors;

    };
}

