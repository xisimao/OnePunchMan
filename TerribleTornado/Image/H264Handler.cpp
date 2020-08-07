#include "H264Handler.h"

using namespace OnePunchMan;

H264Handler::H264Handler(int channelIndex,int count)
	:_channelIndex(channelIndex),_frameIndex(1),_count(count)
{
	_h264File= fopen(StringEx::Combine("../temp/h264_", channelIndex, ".h264").c_str(), "wb");
}

void H264Handler::HandleFrame(int frameIndex,unsigned char* frame, int size)
{
	if (_count > 0 && _frameIndex > _count)
	{
		fclose(_h264File);
		return;
	}
	/*fwrite(frame, 1, size, _h264File);*/
	FILE* file = fopen(StringEx::Combine("../temp/h264_", _channelIndex,"_", frameIndex, ".h264").c_str(), "wb");
	if (file == NULL) {
		return;
	}
	fwrite(frame, 1, size, file);
	fclose(file);
	_frameIndex += 1;
}
