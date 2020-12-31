#include "ImageConvert.h"

using namespace std;
using namespace OnePunchMan;


ImageConvert::ImageConvert(int width, int height, bool useYuvBuffer)
	: _width(width),_height(height)
	, _yuvSize(static_cast<int>(width* height * 1.5)), _yuv_phy_addr(0), _yuvBuffer(NULL)
	, _iveSize(width* height * 3), _ive_phy_addr(0), _iveBuffer(NULL)
	, _bgrBuffer(NULL)
	, _jpgSize(static_cast<int>(tjBufSize(width, height, TJSAMP_422))), _jpgBuffer(NULL)
{
	_bgrBuffer = new unsigned char[_iveSize];
	_jpgBuffer = new unsigned char[_jpgSize];
	if (useYuvBuffer)
	{
#ifndef _WIN32
		if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&_yuv_phy_addr),
			reinterpret_cast<HI_VOID**>(&_yuvBuffer),
			"yuv_buffer",
			NULL,
			_yuvSize) != HI_SUCCESS) {
			LogPool::Error(LogEvent::Decode, "HI_MPI_SYS_MmzAlloc_Cached yuv");
			exit(2);
		}
		if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&_ive_phy_addr),
			reinterpret_cast<HI_VOID**>(&_iveBuffer),
			"ive_buffer",
			NULL,
			_iveSize) != HI_SUCCESS) {
			LogPool::Error(LogEvent::Decode, "HI_MPI_SYS_MmzAlloc_Cached ive");
			exit(2);
		}
#endif // !_WIN32
	}

}

ImageConvert::~ImageConvert()
{
	if (_yuvBuffer != NULL)
	{
#ifndef _WIN32
		HI_MPI_SYS_MmzFree(_yuv_phy_addr, reinterpret_cast<HI_VOID*>(_yuvBuffer));
		HI_MPI_SYS_MmzFree(_ive_phy_addr, reinterpret_cast<HI_VOID*>(_iveBuffer));
#endif // !_WIN32
	}
	delete[] _bgrBuffer;
	delete[] _jpgBuffer;
}

int ImageConvert::GetYuvSize()
{
	return _yuvSize;
}

unsigned char* ImageConvert::GetIveBuffer()
{
	return _iveBuffer;
}

unsigned char* ImageConvert::GetBgrBuffer()
{
	return _bgrBuffer;
}

void ImageConvert::YuvToIve(const unsigned char* yuvBuffer)
{
	if (_yuvBuffer == NULL)
	{
		return;
	}
	memcpy(_yuvBuffer, yuvBuffer, _yuvSize);
#ifndef _WIN32
	IVE_IMAGE_S yuv_image_list;
	IVE_IMAGE_S bgr_image_list;

	IVE_HANDLE ive_handle;
	IVE_CSC_CTRL_S ive_csc_ctrl = { IVE_CSC_MODE_PIC_BT709_YUV2RGB };
	int hi_s32_ret = HI_SUCCESS;

	yuv_image_list.enType = IVE_IMAGE_TYPE_YUV420SP;
	yuv_image_list.u32Height = _height;
	yuv_image_list.u32Width = _width;
	yuv_image_list.au64PhyAddr[0] = _yuv_phy_addr;
	yuv_image_list.au64PhyAddr[1] = yuv_image_list.au64PhyAddr[0] + yuv_image_list.u32Width * yuv_image_list.u32Height;
	yuv_image_list.au32Stride[0] = yuv_image_list.u32Width;
	yuv_image_list.au32Stride[1] = yuv_image_list.u32Width;

	yuv_image_list.au64VirAddr[0] = reinterpret_cast<HI_U64>(_yuvBuffer);
	yuv_image_list.au64VirAddr[1] = yuv_image_list.au64VirAddr[0] + yuv_image_list.u32Width * yuv_image_list.u32Height;

	bgr_image_list.enType = IVE_IMAGE_TYPE_U8C3_PLANAR;
	bgr_image_list.u32Height = _height;
	bgr_image_list.u32Width = _width;
	bgr_image_list.au64PhyAddr[0] = _ive_phy_addr;
	bgr_image_list.au64PhyAddr[1] = bgr_image_list.au64PhyAddr[0] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64PhyAddr[2] = bgr_image_list.au64PhyAddr[1] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64VirAddr[0] = reinterpret_cast<HI_U64>(_iveBuffer);
	bgr_image_list.au64VirAddr[1] = bgr_image_list.au64VirAddr[0] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au64VirAddr[2] = bgr_image_list.au64VirAddr[1] + bgr_image_list.u32Height * bgr_image_list.u32Width;
	bgr_image_list.au32Stride[0] = bgr_image_list.u32Width;
	bgr_image_list.au32Stride[1] = bgr_image_list.u32Width;
	bgr_image_list.au32Stride[2] = bgr_image_list.u32Width;

	hi_s32_ret = HI_MPI_IVE_CSC(&ive_handle, &yuv_image_list, &bgr_image_list, &ive_csc_ctrl, HI_TRUE);
	if (HI_SUCCESS != hi_s32_ret) {
		LogPool::Error(LogEvent::Decode, "HI_MPI_IVE_CSC", StringEx::ToHex(hi_s32_ret));
		return;
	}
	HI_BOOL ive_finish = HI_FALSE;
	hi_s32_ret = HI_SUCCESS;
	do {
		hi_s32_ret = HI_MPI_IVE_Query(ive_handle, &ive_finish, HI_TRUE);
	} while (HI_ERR_IVE_QUERY_TIMEOUT == hi_s32_ret);

	if (HI_SUCCESS != hi_s32_ret) {
		LogPool::Error(LogEvent::Decode, "HI_MPI_IVE_Query", StringEx::ToHex(hi_s32_ret));
		return;
	}
#endif // !_WIN32
}

void ImageConvert::IveToBgr(const unsigned char* iveBuffer, int width, int height)
{
	if (iveBuffer == NULL)
	{
		return;
	}
	const unsigned char* b = iveBuffer;
	const unsigned char* g = b + width * height;
	const unsigned char* r = g + width * height;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			_bgrBuffer[(j * width + i) * 3 + 0] = b[j * width + i];
			_bgrBuffer[(j * width + i) * 3 + 1] = g[j * width + i];
			_bgrBuffer[(j * width + i) * 3 + 2] = r[j * width + i];
		}
	}
}

int ImageConvert::BgrToJpg(const unsigned char* bgrBuffer, int width, int height)
{
	//jpg×ª»»
	tjhandle jpgHandle = tjInitCompress();
	unsigned long tempJpgSize = _jpgSize;
	tjCompress2(jpgHandle, bgrBuffer, width, 0, height, TJPF_BGR, &_jpgBuffer, &tempJpgSize, TJSAMP_422, 10, TJFLAG_NOREALLOC);
	tjDestroy(jpgHandle);
	return static_cast<int>(tempJpgSize);
}

void ImageConvert::BgrToBmp(const unsigned char* bgrBuffer, int width, int height,const std::string& filePath)
{
	FILE* fw = NULL;
	if ((fw = fopen(filePath.c_str(), "wb")) == NULL) {
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
	m_BMPHeader.imageSize = height * width * 3 + header_size;
	m_BMPHeader.startPosition = header_size;

	m_BMPInfoHeader.Length = sizeof(InfoHead);
	m_BMPInfoHeader.width = width;
	//BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
	m_BMPInfoHeader.height = -height;
	m_BMPInfoHeader.colorPlane = 1;
	m_BMPInfoHeader.bitColor = 24;
	m_BMPInfoHeader.realSize = header_size;

	fwrite(bfType, 1, sizeof(bfType), fw);
	fwrite(&m_BMPHeader, 1, sizeof(BmpHead), fw);
	fwrite(&m_BMPInfoHeader, 1, sizeof(InfoHead), fw);
	//BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
	//It saves pixel data in Little Endian

	fwrite(bgrBuffer, 1, width * height * 3, fw);
	fclose(fw);
}

string ImageConvert::JpgToBase64(const unsigned char* jpgBuffer, int jpgSize)
{
	string jpgBase64("data:image/jpg;base64,");
	StringEx::ToBase64String(jpgBuffer, jpgSize, &jpgBase64);
	return jpgBase64;
}

void ImageConvert::BufferToFile(const unsigned char* buffer, int size, const string& filePath)
{
	FILE* fw = fopen(filePath.c_str(), "wb");
	if (fw != NULL)
	{
		fwrite(buffer, 1, size, fw);
		fclose(fw);
	}
}

void ImageConvert::IveToBmp(const unsigned char* iveBuffer, int width, int height, const string& filePath)
{
	IveToBgr(iveBuffer, width, height);
	BgrToBmp(_bgrBuffer, width, height, filePath);
}

void ImageConvert::IveToJpgFile(const unsigned char* iveBuffer, int width, int height, const string& filePath)
{
	IveToBgr(iveBuffer, width, height);
	int jpgSize=BgrToJpg(_bgrBuffer, width, height);
	BufferToFile(_jpgBuffer, jpgSize,filePath);
}

string ImageConvert::IveToJpgBase64(const unsigned char* iveBuffer, int width, int height)
{
	IveToBgr(iveBuffer, width, height);
	int jpgSize = BgrToJpg(_bgrBuffer, width, height);
	return JpgToBase64(_jpgBuffer, jpgSize);
}

void ImageConvert::BgrToJpgFile(const unsigned char* bgrBuffer, int width, int height, const string& filePath)
{
	int jpgSize = BgrToJpg(bgrBuffer, width, height);
	BufferToFile(_jpgBuffer, jpgSize, filePath);
}