#include "YUV420SPHandler.h"

using namespace OnePunchMan;

YUV420SPHandler::YUV420SPHandler(int count)
	:_index(0), _count(count)
{

}

void YUV420SPHandler::HandleFrame(unsigned char* yuv,int width, int height,int packetIndex)
{
	if (_index < _count)
	{
		FILE* file = NULL;
		if ((file = fopen(StringEx::Combine("yuv420sp_", packetIndex, ".yuv").c_str(), "wb")) == NULL) {
			return;
		}
		fwrite(yuv, 1,static_cast<int>(width * height * 1.5), file);
		fclose(file);
		_index += 1;
	}
}
