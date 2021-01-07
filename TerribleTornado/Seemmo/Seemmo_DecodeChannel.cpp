#include "Seemmo_DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;

Seemmo_DecodeChannel::Seemmo_DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel)
	:DecodeChannel(channelIndex,loginId,encodeChannel)
	, _yuvHasData(false), _image(DestinationWidth, DestinationHeight, true)
	,_finished(false), _tempTaskId(0), _tempFrameIndex(0), _tempFrameSpan(0)
{
}

bool Seemmo_DecodeChannel::Decode(unsigned char* data,unsigned int size, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan)
{
#ifdef _WIN32
	return true;
#else
	int hi_s32_ret = HI_SUCCESS;
	if (data != NULL)
	{
		VDEC_STREAM_S stStream;
		stStream.u64PTS = 0;
		unsigned long long temp = frameIndex;
		stStream.u64PTS |= temp;
		temp = taskId;
		stStream.u64PTS |= (temp << 32);
		temp = frameSpan;
		stStream.u64PTS |= (temp << 40);

		stStream.pu8Addr = data;
		stStream.u32Len = size;
		stStream.bEndOfFrame = HI_TRUE;
		stStream.bEndOfStream = HI_FALSE;
		stStream.bDisplay = HI_TRUE;
		hi_s32_ret = HI_MPI_VDEC_SendStream(_channelIndex - 1, &stStream, 0);
		if (HI_SUCCESS != hi_s32_ret) 
		{
			LogPool::Error(LogEvent::Decode,"HI_MPI_VDEC_SendStream", _channelIndex, StringEx::ToHex(hi_s32_ret));
			return false;
		}
	}

	while (true)
	{
		VIDEO_FRAME_INFO_S frame;
		hi_s32_ret = HI_MPI_VPSS_GetChnFrame(_channelIndex - 1, 0, &frame, 0);
		if (hi_s32_ret == HI_SUCCESS)
		{
			_totalFrameCount += 1;
			if (_encodeChannel != NULL)
			{
				_encodeChannel->PushFrame(_channelIndex, &frame);
			}
			while (!SetFrame(&frame)
				&& _syncDetect)
			{
				this_thread::sleep_for(chrono::milliseconds(10));
			}
			HI_MPI_VPSS_ReleaseChnFrame(_channelIndex - 1, 0, &frame);		
		}
		else if (hi_s32_ret == HI_ERR_VPSS_BUF_EMPTY)
		{
			break;
		}
		else
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_VPSS_GetChnFrame", _channelIndex, StringEx::ToHex(hi_s32_ret));
			return false;
		}
	}
	//结束帧强制设置
	if (data == NULL)
	{
		while (true)
		{
			if (SetFrame(NULL))
			{
				break;
			}
			else
			{
				this_thread::sleep_for(chrono::milliseconds(10));
			}
		}
	}
	return true;
#endif // _WIN32

}

#ifndef _WIN32
bool Seemmo_DecodeChannel::SetFrame(VIDEO_FRAME_INFO_S* frame)
{
	if (_yuvHasData)
	{
		return false;
	}
	else
	{
		_handleFrameCount = 1;
		if (frame!=NULL)
		{
			unsigned char* yuv = reinterpret_cast<unsigned char*>(HI_MPI_SYS_Mmap(frame->stVFrame.u64PhyAddr[0], _image.GetYuvSize()));
			_image.YuvToIve(yuv);
			HI_MPI_SYS_Munmap(reinterpret_cast<HI_VOID*>(yuv), _image.GetYuvSize());
			_tempTaskId = frame->stVFrame.u64PTS >> 32 & 0xFF;
			_tempFrameIndex = frame->stVFrame.u64PTS & 0xFFFFFFFF;
			_tempFrameSpan = frame->stVFrame.u64PTS >> 40 & 0xFF;
		}
		_finished = frame==NULL;
		_yuvHasData = true;
		return true;
	}
}

#endif // !_WIN32

FrameItem Seemmo_DecodeChannel::GetIve()
{
	FrameItem item;
	if (_yuvHasData)
	{
		item.TaskId = _tempTaskId;
		item.FrameIndex = _tempFrameIndex;
		item.FrameSpan = _tempFrameSpan;
		item.Finished = _finished;
		//只送出一次结束
		if (_finished)
		{
			_finished = false;
		}
		else
		{
			item.IveBuffer = _image.GetIveBuffer();
		}
		_yuvHasData = false;
	}
	return item;
}
