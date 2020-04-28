#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "BGR24Handler.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

namespace OnePunchMan
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
	class FFmpegChannel :public ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: inputUrl 输入url
		* @param: outputUrl 输出url
		* @param: debug 是否处于调试模式,处于调试模式不循环播放文件
		*/
		FFmpegChannel(const std::string& inputUrl, const std::string& outputUrl, bool debug);

		/**
		* @brief: 析构函数
		*/
		virtual ~FFmpegChannel() {}

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
		virtual bool InitDecoder();
		
		/**
		* @brief: 卸载解码器
		*/
		virtual void UninitDecoder();

		/**
		* @brief: 解码
		* @param: packet 视频帧
		* @param: packetIndex 视频帧序号
		* @return: 解码成功返回true，否则返回false，解码失败会结束线程
		*/
		virtual bool Decode(const AVPacket* packet, int packetIndex);

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

		//是否处于调试模式
		bool _debug;

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
		//当前视频状态
		ChannelStatus _channelStatus;

		//debug
		//解码相关
		AVCodecContext* _decodeContext;
		//解码后的yuv
		AVFrame* _yuvFrame;
		//bgr
		AVFrame* _bgrFrame;
		//bgr宽度
		int _bgrWidth;
		//bgr高度
		int _bgrHeight;
		//bgr字节流
		uint8_t* _bgrBuffer;
		//yuv转bgr
		SwsContext* _bgrSwsContext;
		//bgr写入bmp
		BGR24Handler _bgrHandler;
	};

}

