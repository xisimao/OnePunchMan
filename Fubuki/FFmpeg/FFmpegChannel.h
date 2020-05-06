#pragma once
#include <string>
#include <bitset>

#include "Thread.h"

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
		None,
		//����
		Normal,
		//�����ʼ������
		InputError,
		//�����ʼ������
		OutputError,
		//��������ʼ������
		DecoderError,
		//��ȡ��Ƶ֡����
		ReadError,
		//�������
		DecodeError
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
		* @param: inputUrl ����url
		* @param: outputUrl ���url
		* @param: debug �Ƿ��ڵ���ģʽ,���ڵ���ģʽ��ѭ�������ļ�
		*/
		FFmpegChannel(const std::string& inputUrl, const std::string& outputUrl, bool debug);

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
		* @brief: ��ȡ����ͨ����ַ
		* @return: ����ͨ����ַ
		*/
		const std::string& InputUrl() const;

		/**
		* @brief: ��ȡͨ��״̬
		* @return: ͨ��״̬
		*/
		ChannelStatus Status();

		/**
		* @brief: ��ȡ����֡���
		* @return: ����֡���
		*/
		int PacketSpan();

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
		* @return: ��ʼ���ɹ�����ture�����򷵻�false����ʼ��ʧ�ܻ�����߳�
		*/
		virtual bool InitDecoder();

		/**
		* @brief: ж�ؽ�����
		*/
		virtual void UninitDecoder();

		/**
		* @brief: ����
		* @param: packet ��Ƶ֡
		* @param: packetIndex ��Ƶ֡���
		* @return: ����ɹ�����Handle���Թ�����Ski�����򷵻�Error������ʧ�ܻ�����߳�
		*/
		virtual DecodeResult Decode(const AVPacket* packet, int packetIndex);

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

		//�Ƿ��ڵ���ģʽ
		bool _debug;

	private:
		/**
		* @brief: ��ʼ����Ƶ��ȡ
		* @return: ��Ƶ״̬
		*/
		ChannelStatus Init();

		/**
		* @brief: ж����Ƶ��ȡ
		*/
		void Uninit();

		//������Ƶ��ʼ������
		AVDictionary* _options;
		//��ǰ��Ƶ״̬
		ChannelStatus _channelStatus;
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
		int _lastPacketIndex;
		//���δ���֡�ļ��
		int _packetSpan;
	};

}

