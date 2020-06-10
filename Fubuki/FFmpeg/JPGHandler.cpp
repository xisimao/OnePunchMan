#include "JPGHandler.h"

using namespace OnePunchMan;

JPGHandler::JPGHandler(int count)
	:_index(0), _count(count)
{

}

void JPGHandler::HandleFrame(unsigned char* jpgBuffer, int jpgSize, int frameIndex)
{
	if (_count > 0 && _index >= _count)
	{
		return;
	}
	FILE* fw = NULL;
	if ((fw = fopen(StringEx::Combine("../images/jpg_", frameIndex, ".jpg").c_str(), "wb")) == NULL) {
		return;
	}

	fwrite(jpgBuffer, 1, jpgSize, fw);
	fclose(fw);
	_index += 1;
}