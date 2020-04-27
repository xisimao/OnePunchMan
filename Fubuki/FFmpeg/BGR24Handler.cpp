#include "BGR24Handler.h"

using namespace OnePunchMan;

BGR24Handler::BGR24Handler(int count)
	:_index(0),_count(count)
{

}

void BGR24Handler::HandleFrame(unsigned char* bgr24, int width, int height,int packetIndex)
{
	if (_index >= _count)
	{
		return;
	}
	FILE* fw = NULL;
	if ((fw = fopen(StringEx::Combine("bgr24_", packetIndex, ".bmp").c_str(), "wb")) == NULL) {
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

	BmpHead m_BMPHeader = { 0 };
	InfoHead  m_BMPInfoHeader = { 0 };
	char bfType[2] = { 'B','M' };
	int header_size = sizeof(bfType) + sizeof(BmpHead) + sizeof(InfoHead);
	m_BMPHeader.imageSize = height*width*3 + header_size;
	m_BMPHeader.startPosition = header_size;

	m_BMPInfoHeader.Length = sizeof(InfoHead);
	m_BMPInfoHeader.width = width;
	//BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
	m_BMPInfoHeader.height = -height;
	m_BMPInfoHeader.colorPlane = 1;
	m_BMPInfoHeader.bitColor = 24;
	m_BMPInfoHeader.realSize = header_size;

	fwrite(bfType, 1, sizeof(bfType),fw);
	fwrite(&m_BMPHeader, 1,sizeof(BmpHead),fw);
	fwrite(&m_BMPInfoHeader,1, sizeof(InfoHead),fw);
	//BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
	//It saves pixel data in Little Endian

	fwrite(bgr24, 1,width*height*3,fw);
	fclose(fw);
	_index += 1;
}

void BGR24Handler::WriteJpg(unsigned char* jpg, int size, int packetIndex)
{
	if (_index >= _count)
	{
		return;
	}
	FILE* fw = NULL;
	if ((fw = fopen(StringEx::Combine("jpg_", packetIndex, ".jpg").c_str(), "wb")) == NULL) {
		return;
	}
	fwrite(jpg, 1,size, fw);
	fclose(fw);
	_index += 1;
}