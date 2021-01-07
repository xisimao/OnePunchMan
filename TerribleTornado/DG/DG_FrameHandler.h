#pragma once
#include "TrafficData.h"
#include "FlowDetector.h"
#include "EventDetector.h"
#include "LogPool.h"
#include "ImageConvert.h"

#ifndef _WIN32
#include "dg_sdk_platform.h"
#include "mpi_vpss.h"
#include "mpi_ive.h"
#include "mpi_sys.h"
#endif // !_WIN32

namespace OnePunchMan
{
	class DG_FrameHandler
	{
	public:
		DG_FrameHandler(int channelIndex,int width,int height,FlowDetector* detector);

		static bool Init();

		/**
		* 将下一帧视频写入到bmp
		*/
		void WriteBmp();

		void UpdateChannel(const TrafficChannel& channel);

		bool Finish(int channelIndex);

#ifndef _WIN32
		bool HandleFrame(int channelIndex, VIDEO_FRAME_INFO_S* frame);

		static void Recogn(vega::hisi::DG_STREAM_ID stream_id, const vega::hisi::DG_CAPTURE_RESULT_S& capture_result);

		static void Detect(DgError error, vega::hisi::DG_STREAM_ID stream_id, const vega::hisi::DG_FRAME_RESULT_S& frame_result);

		static void Free(vega::hisi::DG_STREAM_ID stream_id, const std::vector<vega::hisi::DG_FRAME_INPUT_S>& free_frames);

#endif // !_WIN32
	private:
		//sdk是否初始化成功
		static bool Inited;

		//通道序号
		int _channelIndex;
		unsigned long long _streamId;
		//解码后视频宽度
		int _width;
		//解码后视频高度
		int _height;
		//是否需要写入下一帧到bmp
		bool _writeBmp;
		//流量检测
		FlowDetector* _detector;
		//图像转换
		ImageConvert _image;
	};

}


