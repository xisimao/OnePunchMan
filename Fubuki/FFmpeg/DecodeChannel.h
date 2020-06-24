#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "InputHandler.h"
#include "OutputHandler.h"
#include "BGR24Handler.h"

namespace OnePunchMan
{
	//解码结果
	enum class DecodeResult
	{
		Handle,
		Skip,
		Error
	};

	//视频帧读取线程
	class DecodeChannel :public ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: channelIndex 通道序号
		*/
		DecodeChannel(int channelIndex);

		/**
		* @brief: 析构函数
		*/
		virtual ~DecodeChannel() {}

		/**
		* @brief: 初始化ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* @brief: 卸载ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* @brief: 获取通道地址
		* @param: inputUrl 视频源地址
		* @param: outputUrl rtmp输出地址		
		* @param: loop 是否循环播放
		* @return: 当前任务编号
		*/
		unsigned char UpdateChannel(const std::string& inputUrl, const std::string& outputUrl, bool loop);

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

		//解码后的视频宽度
		static const int DestinationWidth;
		//解码后的视频高度
		static const int DestinationHeight;
	
	protected:
		void StartCore();

		/**
		* @brief: 初始化解码器
		* @param: inputUrl 输入url
		* @return: 初始化成功返回true
		*/
		virtual ChannelStatus InitDecoder(const std::string& inputUrl);

		/**
		* @brief: 卸载解码器
		*/
		virtual void UninitDecoder();

		/**
		* @brief: 解码
		* @param: packet 视频帧
		* @param: taskId 任务号
		* @param: frameIndex 视频帧序号
		* @param: frameSpan 两帧的间隔时间(毫秒)
		* @return: 解码成功返回Handle，略过返回Ski，否则返回Error，解码失败会结束线程
		*/
		virtual DecodeResult Decode(const AVPacket* packet, unsigned char taskId,unsigned int frameIndex,unsigned char frameSpan);

		//通道序号
		int _channelIndex;

	private:
		//重连时间(秒)
		static const int ConnectSpan;

		//视频状态同步锁
		std::mutex _mutex;
		//视频输入相关
		std::string _inputUrl;
		InputHandler _inputHandler;

		//视频输出相关
		std::string _outputUrl;
		OutputHandler _outputHandler;

		//任务号
		unsigned char _taskId;
		//当前视频状态
		ChannelStatus _channelStatus;
		//是否循环播放
		bool _loop;

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
		//上一次处理帧的序号
		unsigned int _lastframeIndex;
		//处理帧的间隔
		unsigned int _handleSpan;
		//bgr写bmp
		BGR24Handler _bgrHandler;
	};

}

