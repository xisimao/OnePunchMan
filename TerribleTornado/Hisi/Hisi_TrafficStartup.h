#pragma once
#include "TrafficStartup.h"
#include "Hisi_DecodeChannel.h"

namespace OnePunchMan
{
    //流量系统启动线程
    class Hisi_TrafficStartup :public TrafficStartup
    {
    public:

        /**
        * 构造函数
        */
        Hisi_TrafficStartup();

        /**
        * 启动系统
        */
        void Start();

    protected:
        /**
        * 供子类实现的截图
        * @param channelIndex 通道序号
        */
        void Screenshot(int channelIndex);

    };
}

