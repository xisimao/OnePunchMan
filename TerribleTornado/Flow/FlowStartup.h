#pragma once
#include "FlowData.h"
#include "FlowDetector.h"
#include "DG_FrameHandler.h"
#include "TrafficStartup.h"

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class FlowStartup : public TrafficStartup
    {
    public:
        /**
        * ���캯��
        */
        FlowStartup();

        /**
        * ��������
        */
        ~FlowStartup();

        void Update(HttpReceivedEventArgs* e);

    protected:
        void UpdateDb();

        void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes,std::vector<FrameHandler*>* handlers, std::vector<TrafficDetector*>* detectors);

        void InitChannels();

        std::string GetChannelJson(const std::string& host, int channelIndex);

        void SetDevice(HttpReceivedEventArgs* e);

        void SetChannel(HttpReceivedEventArgs* e);

        bool DeleteChannel(int channelIndex);

    private:
        /**
        * ��ȡͨ���ļ�ⱨ��
        * @param channelIndex ͨ�����
        * @param e http��Ϣ�����¼�����
        */
        void GetReport(int channelIndex, HttpReceivedEventArgs* e);

        std::string CheckFlowChannel(FlowChannel* channel);

        //֡������
        std::vector<DG_FrameHandler*> _handlers;
        //������⼯��
        std::vector<FlowDetector*> _detectors;
        //���ݺϲ�
        DataMergeMap* _merge;

    };
}

