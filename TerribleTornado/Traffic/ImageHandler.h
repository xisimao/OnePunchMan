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
		//bgr�ֽ�������
		int _bgrSize;
		//bgr�ֽ���
		unsigned char* _bgrBuffer;
		//jpg�ֽ�������
		int _jpgSize;
		//jpg�ֽ���
		unsigned char* _jpgBuffer;
	
	};
}

