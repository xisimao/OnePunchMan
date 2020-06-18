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
		//正常
		Normal=1,
		//无法打开视频源
		InputError=2,
		//Rtmp输出异常
		OutputError=3,
		//解码器异常
		DecoderError=4,
		//无法读取视频数据
		ReadError=5,
		//解码错误
		DecodeError=6,
		//准备循环播放
		ReadEOF_Restart =7,
		//文件播放结束
		ReadEOF_Stop=8,
		//正在初始化
		Init = 9
	};

	//解码结果
	enum class DecodeResult
	{
		Handle,
		Skip,
		Error
	};
	//视频帧读取线程
	class FFmpegChannel :public ThreadObject
	{
	public:
		/**
		* @brief: 构造函数
		* @param: channelIndex 通道序号
		*/
		FFmpegChannel(int channelIndex);

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
		int SourceWidth() const;

		/**
		* @brief: 获取输出视频输入高度
		* @return: 视频输入高度
		*/
		int SourceHeight() const;

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
		/**
		* @brief: 初始化视频输入
		* @param: inputUrl 输入url
		* @return: 初始化成功返回true
		*/
		ChannelStatus InitInput(const std::string& inputUrl);

		/**
		* @brief: 初始化视频输出
		* @param: outputUrl 输出url
		* @return: 初始化成功返回true
		*/
		ChannelStatus InitOutput(const std::string& outputUrl);

		/**
		* @brief: 卸载视频输入
		*/
		void UninitInput();

		/**
		* @brief: 卸载视频输出
		*/
		void UninitOutput();

		//重连时间(秒)
		static const int ConnectSpan;

		//视频状态同步锁
		std::mutex _mutex;
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

		//任务号
		unsigned char _taskId;
		//当前视频状态
		ChannelStatus _channelStatus;
		//是否循环播放
		bool _loop;
		//输入视频初始化参数
		AVDictionary* _options;
		//视频源宽度
		int _sourceWidth;
		//视频源高度
		int _sourceHeight;

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

