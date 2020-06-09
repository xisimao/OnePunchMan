#pragma once
#include <string>
#include <bitset>

#include "Thread.h"
#include "BGR24Handler.h"
#include "YUV420SPHandler.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

namespace OnePunchMan
{
	//ͨ��״̬
	enum class ChannelStatus
	{
		//��ʼ��
		Init=0,
		//����
		Normal=1,
		//�����ʼ������
		InputError=2,
		//�����ʼ������
		OutputError=3,
		//��������ʼ������
		DecoderError=4,
		//��ȡ��Ƶ֡����
		ReadError=5,
		//�������
		DecodeError=6,
		//�ļ���ȡ����
		ReadEOF=7
	};

	//������
	enum class DecodeResult
	{
		Handle,
		Skip,
		Error
	};
	//��Ƶ֡��ȡ�߳�
	class FFmpegChannel :public ThreadObject
	{
	public:
		/**
		* @brief: ���캯��
		* @param: debug �Ƿ��ڵ���ģʽ,���ڵ���ģʽ��ѭ�������ļ�
		*/
		FFmpegChannel(bool debug);

		/**
		* @brief: ��������
		*/
		virtual ~FFmpegChannel() {}

		/**
		* @brief: ��ʼ��ffmpeg sdk
		*/
		static void InitFFmpeg();

		/**
		* @brief: ж��ffmpeg sdk
		*/
		static void UninitFFmpeg();

		/**
		* @brief: ��ȡͨ����ַ
		* @param: inputUrl ��ƵԴ��ַ
		* @param: outputUrl rtmp�����ַ		
		*/
		void UpdateChannel(const std::string& inputUrl, const std::string& outputUrl);

		/**
		* @brief: ���ͨ��
		*/
		void ClearChannel();

		/**
		* @brief: ��ȡ����ͨ����ַ
		* @return: ����ͨ����ַ
		*/
		std::string InputUrl();

		/**
		* @brief: ��ȡͨ��״̬
		* @return: ͨ��״̬
		*/
		ChannelStatus Status();

		/**
		* @brief: ��ȡ����֡���
		* @return: ����֡���
		*/
		int FrameSpan();

		/**
		* @brief: ��ȡ�����Ƶ������
		* @return: ��Ƶ������
		*/
		int SourceWidth() const;

		/**
		* @brief: ��ȡ�����Ƶ����߶�
		* @return: ��Ƶ����߶�
		*/
		int SourceHeight() const;

		//��������Ƶ���
		static const int DestinationWidth;
		//��������Ƶ�߶�
		static const int DestinationHeight;

	protected:
		void StartCore();

		/**
		* @brief: ��ʼ��������
		* @param: inputUrl ����url
		* @return: ��ʼ���ɹ�����true
		*/
		virtual ChannelStatus InitDecoder(const std::string& inputUrl);

		/**
		* @brief: ж�ؽ�����
		*/
		virtual void UninitDecoder();

		/**
		* @brief: ����
		* @param: packet ��Ƶ֡
		* @param: frameIndex ��Ƶ֡���
		* @param: frameSpan ��֡�ļ��ʱ��(����)
		* @return: ����ɹ�����Handle���Թ�����Ski�����򷵻�Error������ʧ�ܻ�����߳�
		*/
		virtual DecodeResult Decode(const AVPacket* packet, int frameIndex,int frameSpan);

		//�Ƿ��ڵ���ģʽ
		bool _debug;

	private:
		/**
		* @brief: ��ʼ����Ƶ����
		* @param: inputUrl ����url
		* @return: ��ʼ���ɹ�����true
		*/
		ChannelStatus InitInput(const std::string& inputUrl);

		/**
		* @brief: ��ʼ����Ƶ���
		* @param: outputUrl ���url
		* @return: ��ʼ���ɹ�����true
		*/
		ChannelStatus InitOutput(const std::string& outputUrl);

		/**
		* @brief: ж����Ƶ����
		*/
		void UninitInput();

		/**
		* @brief: ж����Ƶ���
		*/
		void UninitOutput();

		//����ʱ��(��)
		static const int ConnectSpan;

		//��Ƶ״̬ͬ����
		std::mutex _mutex;
		//��Ƶ�������
		std::string _inputUrl;
		AVFormatContext* _inputFormat;
		AVStream* _inputStream;
		int _inputVideoIndex;
		//��Ƶ������
		std::string _outputUrl;
		AVFormatContext* _outputFormat;
		AVStream* _outputStream;
		AVCodecContext* _outputCodec;

		//��ǰ��Ƶ״̬
		ChannelStatus _channelStatus;
		//������Ƶ��ʼ������
		AVDictionary* _options;
		//��ƵԴ���
		int _sourceWidth;
		//��ƵԴ�߶�
		int _sourceHeight;

		//debug
		//�������
		AVCodecContext* _decodeContext;
		//������yuv
		AVFrame* _yuvFrame;
		//bgr
		AVFrame* _bgrFrame;
		//bgr�ֽ���
		uint8_t* _bgrBuffer;
		//yuvתbgr
		SwsContext* _bgrSwsContext;
		//��һ�δ���֡�����
		int _lastframeIndex;
		//���δ���֡�ļ��
		int _frameSpan;
		//bgrдbmp
		BGR24Handler _bgrHandler;
		YUV420SPHandler _yuv420SPHandler;
	};

}

