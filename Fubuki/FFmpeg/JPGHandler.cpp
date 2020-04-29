#include "JPGHandler.h"

using namespace OnePunchMan;

JPGHandler::JPGHandler(int count)
	:_index(0), _count(count)
{

}

void JPGHandler::HandleFrame(unsigned char* jpgBuffer, int jpgSize, long long packetIndex)
{
	if (_count > 0 && _index >= _count)
	{
		return;
	}
	Path::CreateDirectory("../images");
	FILE* fw = NULL;
	if ((fw = fopen(StringEx::Combine("../images/jpg_", packetIndex, ".jpg").c_str(), "wb")) == NULL) {
		return;
	}

	fwrite(jpgBuffer, 1, jpgSize, fw);
	fclose(fw);
	_index += 1;
}