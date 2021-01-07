#include "DG_DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;

DG_DecodeChannel::DG_DecodeChannel(int channelIndex,int loginId, EncodeChannel* encodeChannel, DG_FrameHandler* frameHandler)
	:DecodeChannel(channelIndex,loginId,encodeChannel), _frameHandler(frameHandler)
{


}

bool DG_DecodeChannel::Decode(unsigned char* data, unsigned int size, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan)
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
			LogPool::Error(LogEvent::Decode, "HI_MPI_VDEC_SendStream", _channelIndex, StringEx::ToHex(hi_s32_ret));
			return false;
		}
	}

	while (true)
	{
		VIDEO_FRAME_INFO_S frame;
		hi_s32_ret = HI_MPI_VPSS_GetChnFrame(_channelIndex - 1, 0, &frame, 0);
		if (hi_s32_ret == HI_SUCCESS)
		{
			if (_encodeChannel != NULL)
			{
				_encodeChannel->PushFrame(_channelIndex, &frame);
			}
			if (_frameHandler==NULL||!_frameHandler->HandleFrame(_channelIndex, &frame))
			{
				hi_s32_ret = HI_MPI_VPSS_ReleaseChnFrame(_channelIndex - 1, 0, &frame);
				if (hi_s32_ret != HI_SUCCESS)
				{
					LogPool::Error(LogEvent::Decode, "release", _channelIndex,StringEx::ToHex(hi_s32_ret));
				}
			}
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
	//����֡ǿ������
	if (data == NULL)
	{
		while (true)
		{
			if (_frameHandler==NULL||_frameHandler->Finish(_channelIndex))
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
