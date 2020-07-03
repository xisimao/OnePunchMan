#pragma once
#include "Thread.h"
#include "OutputHandler.h"

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
    class HisiEncodeChannel :public ThreadObject
    {
    public:
        /**
        * @brief: 构造函数
        * @param: encodeCount 编码器总数
        */
        HisiEncodeChannel(int encodeCount);

        /**
        * @brief: 初始化hisi sdk
        * @param: encodeCount 编码器总数
        * @param: width 编码宽度
        * @param: height 编码高度
        * @return: 初始化成功返回true，否则返回false
        */
        static bool InitHisi(int encodeCount, int width, int height);

        /**
        * @brief: 卸载hisi sdk
        * @param: encodeCount 编码器总数
        */
        static void UninitHisi(int encodeCount);

        /**
        * @brief: 开始编码
        * @param: channelIndex 输入通道序号
        * @param: outputUrl 输出视频地址
        * @param: inputUrl 输入视频地址
        * @param: frameCount 输出的总帧数
        * @return: 初始化成功返回编码器的序号，否则返回-1
        */
        int StartEncode(int channelIndex,const std::string& outputUrl,const std::string& inputUrl, int frameCount);

#ifndef _WIN32
        /**
        * @brief: 向编码器推送视频帧
        * @param: channelIndex 通道序号
        * @param: frame 视频帧
        */
        void PushFrame(int channelIndex, VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32

        /**
        * @brief: 停止编码
        * @param: encodeIndex 编码器序号
        */
        void StopEncode(int encodeIndex);

        /**
        * @brief: 获取解码器是否已经输出了足够的视频帧
        * @param: encodeIndex 编码器序号
        */
        bool Finished(int encodeIndex);

    protected:
        void StartCore();

    private:
        //编码器总数
        int _encodeCount;
        //视频输出集合
        std::mutex _mutex;
        std::vector<OutputHandler*> _outputHandlers;
        //解码器对应的输入通道序号
        std::vector<int> _channelIndices;
    };

}
