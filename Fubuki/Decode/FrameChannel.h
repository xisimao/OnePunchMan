#pragma once
#include <string>
#include <bitset>

#include "Thread.h"

extern "C"
{
#include "libavformat/avformat.h"
}

namespace Fubuki
{
	//通道状态
	enum class ChannelStatus
	{
		//已结束
		End,
		//正常
		Normal,
		//输入初始化错误
		InputError,
		//输出初始化错误
		OutputError,
		//解码器初始化错误
		DecoderError,
		//读取视频帧错误
		ReadError,
		//解码错误
		DecodeError
	};

	//视频帧读取线程
	class FrameChannel :public Saitama::ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: inputUrl 输入url
		* @param: outputUrl 输出url
		* @param: loop 是否循环播放
		*/
		FrameChannel(const std::string& inputUrl, const std::string& outputUrl, bool loop);

		/**
		* @brief: 析构函数
		*/
		virtual ~FrameChannel() {}

		/**
		* @brief: 初始化ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* @brief: 卸载ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* @brief: 获取通道状态
		* @return: 通道状态
		*/
		ChannelStatus Status();

	protected:
		void StartCore();

		/**
		* @brief: 初始化解码器
		* @return: 初始化成功返回ture，否则返回false，初始化失败会结束线程
		*/
		virtual bool InitDecoder() { return true; };
		
		/**
		* @brief: 卸载解码器
		*/
		virtual void UninitDecoder() {};

		/**
		* @brief: 解码
		* @param: packet 视频帧
		* @param: packetIndex 视频帧序号
		* @return: 解码成功返回true，否则返回false，解码失败会结束线程
		*/
		virtual bool Decode(const AVPacket* packet, int packetIndex) { return true; }

		//视频输入相关
		std::string _inputUrl;
		AVFormatContext* _inputFormat;
		AVStream* _inputStream;
		int _inputVideoIndex;
		//视频输出相关
		std::string _outputUrl;
		AVFormatContext* _outputFormat;
		AVStream* _outputStream;
		AVCodecContext* _outputCodec;

	private:
		/**
		* @brief: 初始化视频读取
		* @return: 视频状态
		*/
		ChannelStatus Init();

		/**
		* @brief: 卸载视频读取
		*/
		void Uninit();

		//输入视频初始化参数
		AVDictionary* _options;
		//是否循环读取
		bool _loop;
		//当前视频状态
		ChannelStatus _channelStatus;
	};

}

