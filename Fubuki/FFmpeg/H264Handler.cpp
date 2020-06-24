#include "H264Handler.h"

using namespace OnePunchMan;

H264Handler::H264Handler(int count)
	:_frameIndex(1),_count(count)
{

}

void H264Handler::HandleFrame(unsigned char* frame, int size)
{
	if (_count > 0 && _frameIndex > _count)
	{
		return;
	}
	FILE* h264File = fopen(StringEx::Combine("../images/h264_", _frameIndex,".h264").c_str(), "wb");
	fwrite(frame, 1, size, h264File);
	fclose(h264File);
	_frameIndex += 1;
}
