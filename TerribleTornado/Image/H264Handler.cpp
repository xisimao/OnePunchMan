#include "H264Handler.h"

using namespace OnePunchMan;

H264Handler::H264Handler(int channelIndex)
	:_channelIndex(channelIndex)
{
	_h264File= fopen(StringEx::Combine("../temp/h264_", channelIndex, ".h264").c_str(), "wb");
}

void H264Handler::HandleFrame(unsigned char* frame, int size)
{
	fwrite(frame, 1, size, _h264File);
}

void H264Handler::Close()
{
	fclose(_h264File);
}

void H264Handler::HandleFrame(int frameIndex,unsigned char* frame, int size)
{
	FILE* file = fopen(StringEx::Combine("../temp/h264_", _channelIndex,"_", frameIndex, ".h264").c_str(), "wb");
	if (file == NULL) {
		return;
	}
	fwrite(frame, 1, size, file);
	fclose(file);
}
