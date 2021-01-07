#pragma once
#include "TrafficStartup.h"
#include "DG_DecodeChannel.h"
#include "DG_FrameHandler.h"

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class DG_TrafficStartup:public TrafficStartup
    {
    public:

        /**
        * ���캯��
        */
        DG_TrafficStartup();
        /**
        * ����ϵͳ
        */
        void Start();
    protected:
        /**
        * ������ʵ�ֵĸ���ͨ��
        * @param channel ͨ��
        */
        void UpdateChannel(const TrafficChannel& channel);

        /**
        * ������ʵ�ֵĽ�ͼ
        * @param channelIndex ͨ�����
        */
        void Screenshot(int channelIndex);

    private:
        //֡������
        std::vector<DG_FrameHandler*> _handlers;
    };
}

