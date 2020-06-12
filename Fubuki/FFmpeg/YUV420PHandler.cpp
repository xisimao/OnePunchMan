#include "YUV420PHandler.h"

using namespace OnePunchMan;

YUV420PHandler::YUV420PHandler(int count)
	:_index(0), _count(count)
{

}

void YUV420PHandler::HandleFrame(unsigned char* yuv, int width, int height,int frameIndex)
{
	if (_index < _count)
	{
		Path::CreateDirectory("../images");
		FILE* file = fopen(StringEx::Combine("../images/yuv420p_", frameIndex, ".yuv").c_str(), "wb");
		if (file  == NULL) {
			return;
		}
		fwrite(yuv, 1, static_cast<size_t>(width * height * 1.5), file);
		fclose(file);
		_index += 1;
	
	}
}