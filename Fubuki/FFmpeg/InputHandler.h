#pragma once
#include "LogPool.h"

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
		//����
		Normal = 1,
		//�޷�����ƵԴ
		InputError = 2,
		//Rtmp����쳣
		OutputError = 3,
		//�������쳣
		DecoderError = 4,
		//�޷���ȡ��Ƶ����
		ReadError = 5,
		//�������
		DecodeError = 6,
		//׼��ѭ������
		ReadEOF_Restart = 7,
		//�ļ����Ž���
		ReadEOF_Stop = 8,
		//���ڳ�ʼ��
		Init = 9,
		//�����쳣(����)
		Disconnect = 10,
		//ͨ����ͬ��(����)
		NotFoundChannel = 11,
		//������ͬ��(����)
		NotFoundLane = 12,
		//����֡û�д���
		NotHandle = 13
	};

	class InputHandler
	{
	public:
		InputHandler();

		ChannelStatus Init(const std::string& inputUrl);
		
		void Uninit();

		AVFormatContext* FormatContext();
		AVStream* Stream() const;
		int VideoIndex() const;
		int SourceWidth() const;
		int SourceHeight() const;
		unsigned char FrameSpan() const;
	private:
		//��Ƶ�������
		std::string _inputUrl;
		AVFormatContext* _inputFormat;
		AVStream* _inputStream;
		int _inputVideoIndex;
		//��ƵԴ���
		int _sourceWidth;
		//��ƵԴ�߶�
		int _sourceHeight;
		unsigned char _frameSpan;
	};
}


