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
		* ����һ֡��Ƶд�뵽bmp
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
		//sdk�Ƿ��ʼ���ɹ�
		static bool Inited;

		//ͨ�����
		int _channelIndex;
		unsigned long long _streamId;
		//�������Ƶ���
		int _width;
		//�������Ƶ�߶�
		int _height;
		//�Ƿ���Ҫд����һ֡��bmp
		bool _writeBmp;
		//�������
		FlowDetector* _detector;
		//ͼ��ת��
		ImageConvert _image;
	};

}


