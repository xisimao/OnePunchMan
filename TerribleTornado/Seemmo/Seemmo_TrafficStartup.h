#pragma once
#include "TrafficStartup.h"
#include "Seemmo_DetectChannel.h"
#include "Seemmo_RecognChannel.h"
#include "Seemmo_SDK.h"
#include "Seemmo_DecodeChannel.h"

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class Seemmo_TrafficStartup: public TrafficStartup
    {
    public:
        /**
        * ���캯��
        */
        Seemmo_TrafficStartup();

        //����߳�����
        static const int DetectCount;
        //ʶ���߳�����
        static const int RecognCount;

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
        //��Ƶ����̼߳���,keyΪͨ�����
        std::map<int,Seemmo_DetectChannel*> _detects;
        //��Ƶʶ���̼߳���
        std::vector<Seemmo_RecognChannel*> _recogns;

    };
}

