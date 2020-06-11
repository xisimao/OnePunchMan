#include "YUV420SPHandler.h"

using namespace OnePunchMan;

YUV420SPHandler::YUV420SPHandler(int count)
	:_index(0), _count(count)
{

}

void YUV420SPHandler::HandleFrame(const unsigned char* yuv,int width, int height,int frameIndex)
{
	if (_count==-1||_index < _count)
	{
		FILE* file = fopen(StringEx::Combine("../images/yuv420sp_", frameIndex, ".yuv").c_str(), "wb");
		if (file  == NULL) {
			return;
		}
		fwrite(yuv, 1,static_cast<int>(width * height * 1.5), file);
		fclose(file);
		_index += 1;
	}
}
