#pragma once
#include "TrafficStartup.h"
#include "DG_DecodeChannel.h"
#include "DG_FrameHandler.h"

namespace OnePunchMan
{
    //流量系统启动线程
    class DG_TrafficStartup:public TrafficStartup
    {
    public:

        /**
        * 构造函数
        */
        DG_TrafficStartup();
        /**
        * 启动系统
        */
        void Start();
    protected:
        /**
        * 供子类实现的更新通道
        * @param channel 通道
        */
        void UpdateChannel(const TrafficChannel& channel);

        /**
        * 供子类实现的截图
        * @param channelIndex 通道序号
        */
        void Screenshot(int channelIndex);

    private:
        //帧处理集合
        std::vector<DG_FrameHandler*> _handlers;
    };
}

