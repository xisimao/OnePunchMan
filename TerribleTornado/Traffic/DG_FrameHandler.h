#pragma once
#include "FlowData.h"
#include "FlowDetector.h"
#include "FrameHandler.h"

#ifndef _WIN32
#include "dg_sdk_platform.h"
#endif // !_WIN32
namespace OnePunchMan
{
	class DG_FrameHandler:public FrameHandler
	{
	public:
		DG_FrameHandler(int channelIndex,int width,int height,FlowDetector* detector);

		void UpdateChannel(const FlowChannel& channel);

		static bool Init();
#ifndef _WIN32

		bool HandleFrame(VIDEO_FRAME_INFO_S* frame);

		static void Recogn(vega::hisi::DG_STREAM_ID stream_id, const vega::hisi::DG_CAPTURE_RESULT_S& capture_result);

		static void Detect(DgError error, vega::hisi::DG_STREAM_ID stream_id, const vega::hisi::DG_FRAME_RESULT_S& frame_result);

		static void Free(vega::hisi::DG_STREAM_ID stream_id, const std::vector<vega::hisi::DG_FRAME_INPUT_S>& free_frames);

#endif // !_WIN32

	private:
		//sdk是否初始化成功
		static bool Inited;

		unsigned long long _streamId;

		FlowDetector* _detector;

		//帧缓存
#ifndef _WIN32
		std::mutex _frameMutex;
		std::map<int, VIDEO_FRAME_INFO_S> _frames;
#endif // !_WIN32


	};

}


