#include "YUV420PHandler.h"

using namespace Saitama;
using namespace Fubuki;

YUV420PHandler::YUV420PHandler(int count)
	:_index(0), _count(count)
{

}

void YUV420PHandler::HandleFrame(unsigned char* yuv, int width, int height,int packetIndex)
{
	if (_index < _count)
	{
		FILE* file = NULL;
		if ((file = fopen(StringEx::Combine("yuv420p_", packetIndex, ".yuv").c_str(), "wb")) == NULL) {
			return;
		}
		fwrite(yuv, 1, width * height*1.5, file);
		fclose(file);
		_index += 1;
	
	}
}