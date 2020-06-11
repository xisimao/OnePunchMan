#pragma once
#include "LogPool.h"

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

		void AddYuv(const unsigned char* yuvBuffer);

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
		AVFrame* _yuvFrame;
		int _yuvSize;
		int _pts;
	
	};
}


