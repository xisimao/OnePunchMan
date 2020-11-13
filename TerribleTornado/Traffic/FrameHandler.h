#pragma once
#include "LogPool.h"
#include "IVEHandler.h"
#include "TrafficData.h"

#ifndef _WIN32
#include "mpi_vpss.h"
#include "mpi_ive.h"
#include "mpi_sys.h"
#endif // !_WIN32

namespace OnePunchMan
{
	class FrameHandler
	{
	public:
		/**
		* 构造函数
		* @param channelIndex 通道序号
		* @param width 解码后视频宽度
		* @param height 解码后视频高度
		*/
		FrameHandler(int channelIndex,int width, int height);

		virtual ~FrameHandler();

		/**
		* 将下一帧视频写入到bmp
		*/
		void WriteBmp();

#ifndef _WIN32
		/**
		* 供子类实现的处理检测数据
		* @param items 检测数据项集合
		* @param timeStamp 时间戳
		* @param streamId 视频流编号
		* @param taskId 任务编号
		* @param iveBuffer 图片字节流
		* @param frameIndex 帧序号
		* @param frameSpan 帧间隔时间(毫秒)
		* @return 如果返回true表示需要调用者释放帧，否则表示实例负责释放帧
		*/
		virtual bool HandleFrame(VIDEO_FRAME_INFO_S* frame);

	protected:
		/**
		* yuv转ive
		* @return 转换成功返回true,否则返回false
		*/
		bool YuvToIve(VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32

		//通道序号
		int _channelIndex;

		//是否需要写入下一帧到bmp
		bool _writeBmp;

		//图像转换
		//yuv420sp
		int _yuvSize;
		//yuv操作时字节流
		unsigned long long _yuv_phy_addr;
		unsigned char* _yuvBuffer;
		//ive操作时字节流
		int _iveSize;
		unsigned long long _ive_phy_addr;
		unsigned char* _iveBuffer;

	private:
		//解码后视频宽度
		int _width;
		//解码后视频高度
		int _height;
		//ive图像保存
		IVEHandler _iveHandler;

	};
}


