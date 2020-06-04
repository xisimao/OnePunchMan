#include "H264Handler.h"

using namespace OnePunchMan;

H264Handler::H264Handler(int count)
	:_index(0),_count(count)
{
	_h264File = fopen("test.h264", "wb");
}

H264Handler::~H264Handler()
{
	fclose(_h264File);
}

void H264Handler::HandleFrame(unsigned char* frame, int size)
{
	fwrite(frame, 1, size, _h264File);
}

void H264Handler::Close()
{
	if (_h264File != NULL)
	{
		fclose(_h264File);
		_h264File = fopen("test.h264", "wb");
	}
}