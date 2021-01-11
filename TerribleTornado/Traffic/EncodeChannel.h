#pragma once
#include "Thread.h"
#include "H264Cache.h"

#ifndef _WIN32
#include "mpi_venc.h"
#endif // !_WIN32

namespace OnePunchMan
{
    class EncodeChannel :public ThreadObject
    {
    public:
        /**
        * ���캯��
        * @param encodeCount ����������
        */
        EncodeChannel(int encodeCount);

        /**
        * ��ʼ��hisi sdk
        * @param encodeCount ����������
        * @param width ������
        * @param height ����߶�
        * @return ��ʼ���ɹ�����true,���򷵻�false
        */
        static bool InitHisi(int encodeCount, int width, int height);

        /**
        * ж��hisi sdk
        * @param encodeCount ����������
        */
        static void UninitHisi(int encodeCount);

        bool AddOutput(int channelIndex,const std::string& outputUrl, int frameCount);

        void RemoveOutput(int channelIndex, const std::string& outputUrl);

        bool OutputFinished(int channelIndex, const std::string& outputUrl);


#ifndef _WIN32
        /**
        * �������������Ƶ֡
        * @param channelIndex ͨ�����
        * @param frame ��Ƶ֡
        */
        void PushFrame(int channelIndex, VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32

    protected:
        void StartCore();

    private:
        //����������
        int _encodeCount;
        //��Ƶ�����漯��
        std::vector<H264Cache*> _caches;
    };

}
