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
	ImageConvert::IveToBgr(iveBuffer, _width, _height, _bgrBuffer);
	int jpgSize=ImageConvert::BgrToJpg(_bgrBuffer, _width, _height, &_jpgBuffer, _jpgSize);
	string base64("data:image/jpg;base64,");
	StringEx::ToBase64String(_jpgBuffer, jpgSize, &base64);
	JsonSerialization::SerializeValue(_json,StringEx::Combine("image", _imageIndex), base64);
	_count -= 1;
	_imageIndex += 1;
}