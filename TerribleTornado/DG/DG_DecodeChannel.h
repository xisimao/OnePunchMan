#pragma once
#include "DecodeChannel.h"
#include "DG_FrameHandler.h"

namespace OnePunchMan
{
	//视频帧读取线程
	class DG_DecodeChannel :public DecodeChannel
	{
	public:
		/**
		* 构造函数
		* @param channelIndex 通道序号
		* @param loginId 国标登陆id
		* @param encodeChannel 编码线程
		* @param frameHandler 帧处理
		*/
		DG_DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel, DG_FrameHandler* frameHandler);

	protected:
		/**
		* 解码
		* @param packet 视频字节流
		* @param size 视频字节流长度
		* @param taskId 任务号
		* @param frameIndex 视频帧序号
		* @param frameSpan 两帧的间隔时间(毫秒)
		* @return 解码成功返回true，否则返回false
		*/
		bool Decode(unsigned char* buffer, unsigned int size, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan);

	private:
		//处理帧
		DG_FrameHandler* _frameHandler;
	};

}

