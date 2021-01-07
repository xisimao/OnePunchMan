#pragma once
#include "DecodeChannel.h"

namespace OnePunchMan
{
	//��Ƶ֡��ȡ�߳�
	class Seemmo_DecodeChannel :public DecodeChannel
	{
	public:
		/**
		* ���캯��
		* @param channelIndex ͨ�����
		* @param loginId �����½id
		* @param encodeChannel �����߳�
		*/
		Seemmo_DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel);

		/**
		* ��ȡ��ʱ��ŵ�ive����
		* @return ive֡����
		*/
		FrameItem GetIve();

	protected:
		/**
		* ����
		* @param packet ��Ƶ�ֽ���
		* @param size ��Ƶ�ֽ�������
		* @param taskId �����
		* @param frameIndex ��Ƶ֡���
		* @param frameSpan ��֡�ļ��ʱ��(����)
		* @return ����ɹ�����true�����򷵻�false
		*/
		bool Decode(unsigned char* buffer, unsigned int size, unsigned char taskId, unsigned int frameIndex, unsigned char frameSpan);

	private:
#ifndef _WIN32
		/**
		* д����ʱive����
		* @param frame ��Ƶ֡,����NULL��ʾ��ɽ���
		* @return ����true��ʾ�ɹ�д��,����false��ʾ��ǰ�Ѿ���yuv����
		*/
		bool SetFrame(VIDEO_FRAME_INFO_S* frame);
#endif // !_WIN32

		//�첽����
		//yuv�Ƿ�������
		bool _yuvHasData;
		//ͼ��ת��
		ImageConvert _image;
		//��ǰ��Ƶ��ȡ�Ƿ����
		bool _finished;
		//��ǰyuv���ݵ������
		unsigned char _tempTaskId;
		//��ǰyuv���ݵ�֡���
		unsigned int _tempFrameIndex;
		//��ǰyuv���ݵ�֡���ʱ��
		unsigned char _tempFrameSpan;
	};

}

