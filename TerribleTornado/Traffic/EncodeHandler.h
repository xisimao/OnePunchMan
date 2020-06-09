#pragma once
#include "LogPool.h"
#include "YUV420PHandler.h"
#include "YUV420SPHandler.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

namespace OnePunchMan
{
	class EncodeHandler
	{
	public:
		EncodeHandler(std::string* json, const std::string& filePath, int width, int height, int seconds);

		~EncodeHandler();

		bool Finished();

		void AddYuv(unsigned char* yuv420pBuffer);

	private:
		void Uninit();

		static const int FPS;

		std::string* _json;
		std::string _filePath;
		int _width;
		int _height;
		int _count;
		AVCodecContext* _encodeContext;
		AVFormatContext* _outputContext;
		SwsContext* _swsContext;
		AVFrame* _yuv420pFrame;
		int _yuvSize;
		int _pts;
	
	};
}


