#pragma once
#include "DecodeChannel.h"

namespace OnePunchMan
{
	//视频帧读取线程
	class Seemmo_DecodeChannel :public DecodeChannel
	{
	public:
		/**
		* 构造函数
		* @param channelIndex 通道序号
		* @param loginId 国标登陆id
		* @param encodeChannel 编码线程
		*/
		Seemmo_DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel);

		/**
		* 获取临时存放的ive数据
		* @return ive帧数据
		*/
		FrameItem GetIve();

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
#ifndef _WIN32
		/**
		* 写入临时ive数据
		* @param frame 视频帧,传入NULL表示完成解码
		* @return 返回true表示成功写入,返回false表示当前已经有yuv数据
		*/
		bool SetFrame(VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32

		//异步队列
		//yuv是否有数据
		bool _yuvHasData;
		//图像转换
		ImageConvert _image;
		//当前视频读取是否结束
		bool _finished;
		//当前yuv数据的任务号
		unsigned char _tempTaskId;
		//当前yuv数据的帧序号
		unsigned int _tempFrameIndex;
		//当前yuv数据的帧间隔时长
		unsigned char _tempFrameSpan;
	};

}

