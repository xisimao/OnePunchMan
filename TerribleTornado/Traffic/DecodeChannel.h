#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "FFmpegOutput.h"
#include "EncodeChannel.h"
#include "ImageConvert.h"
#include "TrafficData.h"

#ifndef _WIN32
#include "clientsdk.h"
#include "hi_comm_video.h"
#include "hi_buffer.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#endif // !_WIN32

namespace OnePunchMan
{
	//帧数据
	class FrameItem
	{
	public:
		FrameItem()
			:TaskId(0), IveBuffer(NULL), FrameIndex(0), FrameSpan(0), Finished(false)
		{

		}
		//任务号
		unsigned char TaskId;
		//ive字节流
		unsigned char* IveBuffer;
		//帧序号
		unsigned int FrameIndex;
		//帧率
		unsigned char FrameSpan;
		//视频是否已经读取完成
		bool Finished;
	};

	//视频帧读取线程
	class DecodeChannel :public ThreadObject
	{
	public:
		/**
		* 构造函数
		* @param channelIndex 通道序号
		* @param loginId 国标登陆id
		* @param encodeChannel 编码线程
		*/
		DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel);

		virtual ~DecodeChannel()
		{

		}

		/**
		* 初始化ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* 卸载ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* 初始化hisi sdk
		* @param videoCount 通道总数
		* @param blockCount 缓存池数量
		* @return 初始化成功返回true,否则返回false
		*/
		static bool InitHisi(int videoCount,int blockCount);

		/**
		* 卸载hisi sdk
		* @param videoCount 通道总数
		*/
		static void UninitHisi(int videoCount);

		/**
		* 更新视频通道
		* @param channel 视频通道
		* @return 当前任务编号
		*/
		unsigned char UpdateChannel(const TrafficChannel& channel);

		/**
		* 清空通道
		*/
		void ClearChannel();

		/**
		* 获取输入通道地址
		* @return 输入通道地址
		*/
		std::string InputUrl();

		/**
		* 获取通道状态
		* @return 通道状态
		*/
		ChannelStatus Status();

		/**
		* 获取帧率
		* @return 帧率
		*/
		int FrameSpan();

		/**
		* 获取输出视频输入宽度
		* @return 视频输入宽度
		*/
		int SourceWidth();

		/**
		* 获取输出视频输入高度
		* @return 视频输入高度
		*/
		int SourceHeight();

		//解码后的视频宽度
		static const int DestinationWidth;
		//解码后的视频高度
		static const int DestinationHeight;

	protected:
		void StartCore();

		/**
		* 解码
		* @param packet 视频字节流
		* @param size 视频字节流长度
		* @param taskId 任务号
		* @param frameIndex 视频帧序号
		* @param frameSpan 两帧的间隔时间(毫秒)
		* @return 解码成功返回true，否则返回false
		*/
		virtual bool Decode(unsigned char* buffer, unsigned int size, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan)
		{
			return true;
		}

		//构造函数时改变
		//通道序号
		int _channelIndex;
		//国标登陆id
		int _loginId;

		//更新通道时改变
		//视频状态同步锁
		std::mutex _mutex;
		//视频输入地址
		std::string _inputUrl;
		//视频输出地址
		std::string _outputUrl;
		//通道类型
		ChannelType _channelType;
		//当前视频状态
		ChannelStatus _channelStatus;
		//任务号
		unsigned char _taskId;
		//是否循环播放
		bool _loop;
		//是否同步检测，全局检测文件视频时执行同步检测
		bool _syncDetect;

		//线程中改变
		//输入视频
		AVFormatContext* _inputFormat;
		//视频流
		AVStream* _inputStream;
		//视频流序号
		int _inputVideoIndex;
		//视频源宽度
		int _sourceWidth;
		//视频源高度
		int _sourceHeight;
		//输入视频帧间隔时长(毫秒)
		unsigned char _frameSpan;
		//帧数
		unsigned int _frameIndex;
		//总共读取的帧数
		unsigned int _totalFrameCount;
		//处理的帧数
		unsigned int _handleFrameCount;
		//当前处理视频的任务号
		unsigned char _currentTaskId;
		//当前国标播放句柄
		int _playHandler;

		//视频输出
		FFmpegOutput _outputHandler;

		//编码
		EncodeChannel* _encodeChannel;

	private:
		/**
		* 初始化解码器
		* @param playFd 视频句柄
		* @param frameType 帧类型
		* @param buffer 帧字节流
		* @param size 帧字节流长度
		* @param usr 用户自定义数据
		*/
		static void ReceivePacket(int playFd, int frameType, char* buffer, unsigned int size, void* usr);

		/**
		* 初始化视频输入
		* @param inputUrl 视频输入地址
		* @return 视频状态
		*/
		bool InitInput(const std::string& inputUrl);

		/**
		* 结束视频输入
		*/
		void UninitInput();

		//重连时间(豪秒)
		static const int ConnectSpan;
		//国标休眠时间(毫秒)
		static const int GbSleepSpan;
		//最长的处理帧间隔,如果超过这个间隔就会修改通道状态
		static const int MaxHandleSpan;

		
	};

}

