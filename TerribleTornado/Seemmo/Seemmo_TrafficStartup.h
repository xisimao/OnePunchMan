#pragma once
#include "TrafficStartup.h"
#include "Seemmo_DetectChannel.h"
#include "Seemmo_RecognChannel.h"
#include "Seemmo_SDK.h"
#include "Seemmo_DecodeChannel.h"

namespace OnePunchMan
{
    //流量系统启动线程
    class Seemmo_TrafficStartup: public TrafficStartup
    {
    public:
        /**
        * 构造函数
        */
        Seemmo_TrafficStartup();

        //检测线程总数
        static const int DetectCount;
        //识别线程总数
        static const int RecognCount;

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
        //视频检测线程集合,key为通道序号
        std::map<int,Seemmo_DetectChannel*> _detects;
        //视频识别线程集合
        std::vector<Seemmo_RecognChannel*> _recogns;

    };
}

