#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "FFmpegInput.h"
#include "FFmpegOutput.h"
#include "EncodeChannel.h"
#include "BGR24Handler.h"
#include "TrafficData.h"

#ifndef _WIN32
#include "clientsdk.h"
#include "hi_common.h"
#include "hi_buffer.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vpss.h"
#include "hi_comm_avs.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"
#include "hi_comm_hdmi.h"
#include "hi_mipi.h"
#include "hi_comm_hdr.h"
#include "hi_comm_vgs.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#include "mpi_avs.h"
#include "mpi_region.h"
#include "mpi_audio.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "hi_math.h"
#include "hi_sns_ctrl.h"
#include "mpi_hdmi.h"
#include "mpi_hdr.h"
#include "mpi_vgs.h"
#include "mpi_ive.h"
#endif // !_WIN32

namespace OnePunchMan
{
	//解码结果
	enum class DecodeResult
	{
		Handle,
		Skip,
		Error
	};

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
		* @brief: 构造函数
		* @param: channelIndex 通道序号
		* @param: loginId 国标登陆id
		* @param: encodeChannel 编码线程
		*/
		DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel);

		/**
		* @brief: 析构函数
		*/
		~DecodeChannel();

		/**
		* @brief: 初始化hisi sdk
		* @param: videoCount 通道总数
		* @return: 初始化成功返回true，否则返回false
		*/
		static bool InitHisi(int videoCount);

		/**
		* @brief: 卸载hisi sdk
		* @param: videoCount 通道总数
		*/
		static void UninitHisi(int videoCount);

		/**
		* @brief: 获取通道地址
		* @param: inputUrl 视频源地址
		* @param: outputUrl rtmp输出地址		
		* @param: channelType 通道类型		
		* @param: loop 是否循环播放
		* @return: 当前任务编号
		*/
		unsigned char UpdateChannel(const std::string& inputUrl, const std::string& outputUrl, ChannelType channelType, bool loop);

		/**
		* @brief: 清空通道
		*/
		void ClearChannel();


		/**
		* @brief: 获取输入通道地址
		* @return: 输入通道地址
		*/
		std::string InputUrl();

		/**
		* @brief: 获取通道状态
		* @return: 通道状态
		*/
		ChannelStatus Status();

		/**
		* @brief: 获取帧率
		* @return: 帧率
		*/
		int FrameSpan();

		/**
		* @brief: 获取处理帧间隔
		* @return: 处理帧间隔
		*/
		int HandleSpan();

		/**
		* @brief: 获取输出视频输入宽度
		* @return: 视频输入宽度
		*/
		int SourceWidth();

		/**
		* @brief: 获取输出视频输入高度
		* @return: 视频输入高度
		*/
		int SourceHeight();

		/**
		* @brief: 获取临时存放的ive数据
		* @return: ive帧数据
		*/
		FrameItem GetTempIve();

		//解码后的视频宽度
		static const int DestinationWidth;
		//解码后的视频高度
		static const int DestinationHeight;
	
	protected:
		void StartCore();

	private:
		/**
		* @brief: 初始化解码器
		* @param: playFd 视频句柄
		* @param: frameType 帧类型
		* @param: buffer 帧字节流
		* @param: size 帧字节流长度
		* @param: usr 用户自定义数据
		*/
		static void ReceivePacket(int playFd, int frameType, char* buffer, unsigned int size, void* usr);

		/**
		* @brief: 初始化解码器
		* @param: inputUrl 输入url
		* @return: 初始化成功返回true
		*/
		ChannelStatus InitDecoder(const std::string& inputUrl);

		/**
		* @brief: 卸载解码器
		*/
		void UninitDecoder();

		/**
		* @brief: 解码
		* @param: packet 视频帧
		* @param: taskId 任务号
		* @param: frameIndex 视频帧序号
		* @param: frameSpan 两帧的间隔时间(毫秒)
		* @return: 解码成功返回Handle，略过返回Ski，否则返回Error，解码失败会结束线程
		*/
		DecodeResult Decode(const AVPacket* packet, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan);

		/**
		* @brief: 写入临时ive数据
		* @param: taskId 任务编号
		* @param: yuv yuv数据
		* @param: frameIndex 帧序号
		* @param: finished 视频是否读取完成
		* @return: 返回true表示成功写入，返回false表示当前已经有yuv数据
		*/
		bool SetTempIve(unsigned char taskId, const unsigned char* yuv, unsigned int frameIndex, unsigned char frameSpan, bool finished);

		/**
		* @brief: yuv转ive
		* @return: 转换成功返回true，否则返回false
		*/
		bool YuvToIve();

		//重连时间(豪秒)
		static const int ConnectSpan;
		//国标休眠时间(毫秒)
		static const int GbSleepSpan;
		//最长的处理帧间隔，如果超过这个间隔就会修改通道状态
		static const int MaxHandleSpan;

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
		
		//线程中改变
		//视频输入
		FFmpegInput _inputHandler;
		//视频输出
		FFmpegOutput _outputHandler;
		//帧间隔时长(毫秒)
		unsigned char _frameSpan;
		//帧数
		unsigned int _frameIndex;
		//上一次处理帧的序号
		unsigned int _lastframeIndex;
		//处理帧的间隔
		unsigned int _handleSpan;
		//帧时长
		long long _duration;
		//当前处理视频的任务号
		unsigned char _currentTaskId;
		//当前国标播放句柄
		int _playHandler;

		//debug
		//解码相关
		AVCodecContext* _decodeContext;
		//解码后的yuv
		AVFrame* _yuvFrame;
		//bgr
		AVFrame* _bgrFrame;
		//bgr字节流
		uint8_t* _bgrBuffer;
		//yuv转bgr
		SwsContext* _bgrSwsContext;
		//bgr写bmp
		BGR24Handler _bgrHandler;

		//异步队列
		//当前视频读取是否结束
		bool _finished;
		//当前yuv数据的任务号
		unsigned char _tempTaskId;
		//当前yuv数据的帧序号
		unsigned int _tempFrameIndex;
		//当前yuv数据的帧间隔时长
		unsigned char _tempFrameSpan;

		//解码相关
		//yuv420sp
		int _yuvSize;
		//当前yuv字节流是否已经有了yuv数据
		bool _yuvHasValue;
		//yuv操作时字节流
		unsigned long long _yuv_phy_addr;
		uint8_t* _yuvBuffer;
		//ive操作时字节流
		int _iveSize;
		unsigned long long _ive_phy_addr;
		uint8_t* _iveBuffer;

		//国标使用的packet
		AVPacket* _gbPacket;

		//编码
		EncodeChannel* _encodeChannel;
		//是否已经有了i帧
		bool _gotIFrame;

	};

}

