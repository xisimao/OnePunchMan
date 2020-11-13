#include "FrameHandler.h"

using namespace std;
using namespace OnePunchMan;

FrameHandler::FrameHandler(int channelIndex, int width, int height)
	: _channelIndex(channelIndex), _writeBmp(false)
	, _yuvSize(static_cast<int>(width* height * 1.5)), _yuv_phy_addr(0), _yuvBuffer(NULL), _iveSize(width* height * 3), _ive_phy_addr(0), _iveBuffer(NULL)
	, _width(width), _height(height), _iveHandler(TrafficDirectory::FileDir, -1)
{
#ifndef _WIN32
	if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&_yuv_phy_addr),
		reinterpret_cast<HI_VOID**>(&_yuvBuffer),
		"yuv_buffer",
		NULL,
		_yuvSize) != HI_SUCCESS) {
		LogPool::Error(LogEvent::Decode, "HI_MPI_SYS_MmzAlloc_Cached yuv");
	}
	if (HI_MPI_SYS_MmzAlloc_Cached(reinterpret_cast<HI_U64*>(&_ive_phy_addr),
		reinterpret_cast<HI_VOID**>(&_iveBuffer),
		"ive_buffer",
		NULL,
		_iveSize) != HI_SUCCESS) {
		LogPool::Error(LogEvent::Decode, "HI_MPI_SYS_MmzAlloc_Cached ive");
	}
#endif // !_WIN32
}

FrameHandler::~FrameHandler()
{
#ifndef _WIN32
	HI_MPI_SYS_MmzFree(_yuv_phy_addr, reinterpret_cast<HI_VOID*>(_yuvBuffer));
	HI_MPI_SYS_MmzFree(_ive_phy_addr, reinterpret_cast<HI_VOID*>(_iveBuffer));
#endif // !_WIN32
}

void FrameHandler::WriteBmp()
{
	_writeBmp = true;
}

#ifndef _WIN32
bool FrameHandler::HandleFrame(VIDEO_FRAME_INFO_S* frame)
{
	if (_writeBmp && frame != NULL)
	{
		YuvToIve(frame);
		_iveHandler.HandleFrame(_iveBuffer, _width, _height, _channelIndex);
		_writeBmp = false;
	}
	return false;
}

bool FrameHandler::YuvToIve(VIDEO_FRAME_INFO_S* frame)
{
	unsigned char* yuv = reinterpret_cast<unsigned char*>(HI_MPI_SYS_Mmap(frame->stVFrame.u64PhyAddr[0], _yuvSize));
	memcpy(_yuvBuffer, yuv, _yuvSize);
	HI_MPI_SYS_Munmap(reinterpret_cast<HI_VOID*>(yuv), _yuvSize);
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
		return false;
	}
	HI_BOOL ive_finish = HI_FALSE;
	hi_s32_ret = HI_SUCCESS;
	do {
		hi_s32_ret = HI_MPI_IVE_Query(ive_handle, &ive_finish, HI_TRUE);
	} while (HI_ERR_IVE_QUERY_TIMEOUT == hi_s32_ret);

	if (HI_SUCCESS != hi_s32_ret) {
		LogPool::Error(LogEvent::Decode, "HI_MPI_IVE_Query", StringEx::ToHex(hi_s32_ret));
		return false;
	}
	return true;
}
#endif // !_WIN3