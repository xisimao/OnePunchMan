#pragma once
#include "Thread.h"
#include "H264Cache.h"

#ifndef _WIN32
#include "hi_common.h"
#include "hi_buffer.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vpss.h"
#include "hi_comm_avs.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"
#include "hi_comm_hdmi.h"
#include "hi_mipi.h"
#include "hi_comm_hdr.h"
#include "hi_comm_vgs.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#include "mpi_avs.h"
#include "mpi_region.h"
#include "mpi_audio.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "hi_math.h"
#include "hi_sns_ctrl.h"
#include "mpi_hdmi.h"
#include "mpi_hdr.h"
#include "mpi_vgs.h"
#include "mpi_ive.h"
#endif // !_WIN32

namespace OnePunchMan
{
    class EncodeChannel :public ThreadObject
    {
    public:
        /**
        * @brief: ���캯��
        * @param: encodeCount ����������
        */
        EncodeChannel(int encodeCount);

        /**
        * @brief: ��ʼ��hisi sdk
        * @param: encodeCount ����������
        * @param: width ������
        * @param: height ����߶�
        * @return: ��ʼ���ɹ�����true�����򷵻�false
        */
        static bool InitHisi(int encodeCount, int width, int height);

        /**
        * @brief: ж��hisi sdk
        * @param: encodeCount ����������
        */
        static void UninitHisi(int encodeCount);

        bool AddOutput(int channelIndex,const std::string& outputUrl, int frameCount);

        bool OutputFinished(int channelIndex, const std::string& outputUrl);


#ifndef _WIN32
        /**
        * @brief: �������������Ƶ֡
        * @param: channelIndex ͨ�����
        * @param: frame ��Ƶ֡
        */
        void PushFrame(int channelIndex, VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32

    protected:
        void StartCore();

    private:
        //����������
        int _encodeCount;
        std::vector<H264Cache*> _cache;
    };

}
