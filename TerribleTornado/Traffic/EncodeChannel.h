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
        * 构造函数
        * @param encodeCount 编码器总数
        */
        EncodeChannel(int encodeCount);

        /**
        * 初始化hisi sdk
        * @param encodeCount 编码器总数
        * @param width 编码宽度
        * @param height 编码高度
        * @return 初始化成功返回true,否则返回false
        */
        static bool InitHisi(int encodeCount, int width, int height);

        /**
        * 卸载hisi sdk
        * @param encodeCount 编码器总数
        */
        static void UninitHisi(int encodeCount);

        bool AddOutput(int channelIndex,const std::string& outputUrl, int frameCount);

        void RemoveOutput(int channelIndex, const std::string& outputUrl);

        bool OutputFinished(int channelIndex, const std::string& outputUrl);


#ifndef _WIN32
        /**
        * 向编码器推送视频帧
        * @param channelIndex 通道序号
        * @param frame 视频帧
        */
        void PushFrame(int channelIndex, VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32

    protected:
        void StartCore();

    private:
        //编码器总数
        int _encodeCount;
        //视频包缓存集合
        std::vector<H264Cache*> _caches;
    };

}
