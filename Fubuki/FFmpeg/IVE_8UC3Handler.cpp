#include "IVE_8UC3Handler.h"

using namespace std;
using namespace OnePunchMan;

const int IVE_8UC3Handler::HeaderSize = 54;

IVE_8UC3Handler::IVE_8UC3Handler(int count)
	:_index(0), _count(count)
{

}

void IVE_8UC3Handler::HandleFrame(unsigned char* ive, int width, int height, int frameIndex)
{
	if (_count > 0 && _index >= _count)
	{
		return;
	}
	FILE* fw = fopen(StringEx::Combine("../images/channel_", frameIndex, ".bmp").c_str(), "wb");
	if (fw ==NULL) {
		return;
	}
	typedef struct
	{
		int imageSize;
		int blank;
		int startPosition;
	}BmpHead;

	typedef struct
	{
		int  Length;
		int  width;
		int  height;
		unsigned short  colorPlane;
		unsigned short  bitColor;
		int  zipFormat;
		int  realSize;
		int  xPels;
		int  yPels;
		int  colorUse;
		int  colorImportant;
	}InfoHead;

	BmpHead bmpHeader = { 0 };
	InfoHead  bmpInfoHeader = { 0 };
	char type[2] = { 'B','M' };

	bmpHeader.imageSize = width * height * 3 + HeaderSize;
	bmpHeader.startPosition = HeaderSize;

	bmpInfoHeader.Length = sizeof(InfoHead);
	bmpInfoHeader.width = width;
	//BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
	bmpInfoHeader.height = -height;
	bmpInfoHeader.colorPlane = 1;
	bmpInfoHeader.bitColor = 24;
	bmpInfoHeader.realSize = HeaderSize;

	uint8_t* bmp = new uint8_t[width * height * 3 + HeaderSize];

	memcpy(bmp, &type, sizeof(type));
	memcpy(bmp + sizeof(type), &bmpHeader, sizeof(BmpHead));
	memcpy(bmp + sizeof(type) + sizeof(BmpHead), &bmpInfoHeader, sizeof(InfoHead));

	//BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
	//It saves pixel data in Little Endian
	uint8_t* b = ive;
	uint8_t* g = b + width * height;
	uint8_t* r = g + width * height;
	uint8_t* image = bmp + HeaderSize;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			image[(j * width + i) * 3 + 0] = b[j * width + i];
			image[(j * width + i) * 3 + 1] = g[j * width + i];
			image[(j * width + i) * 3 + 2] = r[j * width + i];
		}
	}
	fwrite(bmp, 1, width*height*3+ HeaderSize, fw);
	fclose(fw);
	delete[] bmp;
	_index += 1;
}
