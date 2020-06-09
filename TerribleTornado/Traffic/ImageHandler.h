#pragma once
#include <string>
#include "StringEx.h"
#include "JsonFormatter.h"
#include "ImageConvert.h"

namespace OnePunchMan
{
	class ImageHandler
	{
	public:
		ImageHandler(std::string* json, int count, int width, int height);

		~ImageHandler();

		bool Finished();

		void AddIve(const unsigned char* iveBuffer);

	private:
		std::string* _json;
		int _count;
		int _width;
		int _height;
		int _imageIndex;
		//bgr字节流长度
		int _bgrSize;
		//bgr字节流
		unsigned char* _bgrBuffer;
		//jpg字节流长度
		int _jpgSize;
		//jpg字节流
		unsigned char* _jpgBuffer;
	
	};
}

