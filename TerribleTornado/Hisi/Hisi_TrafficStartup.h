#pragma once
#include "TrafficStartup.h"
#include "Hisi_DecodeChannel.h"

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class Hisi_TrafficStartup :public TrafficStartup
    {
    public:

        /**
        * ���캯��
        */
        Hisi_TrafficStartup();

        /**
        * ����ϵͳ
        */
        void Start();

    protected:
        /**
        * ������ʵ�ֵĽ�ͼ
        * @param channelIndex ͨ�����
        */
        void Screenshot(int channelIndex);

    };
}

