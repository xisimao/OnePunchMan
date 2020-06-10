#include "ImageHandler.h"

using namespace std;
using namespace OnePunchMan;

ImageHandler::ImageHandler(string* json,int count,int width,int height)
	:_json(json),_count(count), _width(width),_height(height), _imageIndex(1)
{
	_jpgSize = static_cast<int>(tjBufSize(width, height, TJSAMP_422));
	_jpgBuffer = tjAlloc(_jpgSize);
	_bgrBuffer = new unsigned char[width * height * 3];
}

ImageHandler::~ImageHandler()
{
	tjFree(_jpgBuffer);
	delete[] _bgrBuffer;
}

bool ImageHandler::Finished()
{
	return _count <= 0;
}

void ImageHandler::AddIve(const unsigned char* iveBuffer)
{
	if (_count <= 0)
	{
		return;
	}
	string image;
	ImageConvert::IveToJpgBase64(iveBuffer, _width,_height, _bgrBuffer, &image, _jpgBuffer, _jpgSize);
	JsonSerialization::SerializeValue(_json,StringEx::Combine("image", _imageIndex), image);
	_count -= 1;
	_imageIndex += 1;
}