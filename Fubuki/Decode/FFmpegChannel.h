#pragma once
#include "FrameChannel.h"
#include "H264Handler.h"
#include "YUV420PHandler.h"
#include "BGR24Handler.h"

extern "C"
{
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

namespace Fubuki
{
	//FFmpeg����
	class FFmpegChannel :public FrameChannel
	{
	public:
		/**
		* @brief: ���캯��
		* @param: inputUrl ����url
		* @param: outputUrl ���url
		* @param: loop �Ƿ�ѭ������
		*/
		FFmpegChannel(const std::string& inputUrl, const std::string& outputUrl, bool loop);

	protected:
		bool InitDecoder();

		void UninitDecoder();

		bool Decode(const AVPacket* packet, int packetIndex);

	private:
		//�������
		AVCodecContext* _decodeContext;
		//������yuv
		AVFrame* _yuvFrame;
		//bgr
		AVFrame* _bgrFrame;
		uint8_t* _bgrBuffer;
		SwsContext* _bgrSwsContext;

		//h264д��
		H264Handler* _h264Handler;
		//yuv420pд��
		YUV420PHandler* _yuvHandler;
		//bgr24д��
		BGR24Handler* _bgrHandler;
	
	};

}

