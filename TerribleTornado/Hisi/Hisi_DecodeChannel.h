#pragma once
#include "DecodeChannel.h"

namespace OnePunchMan
{
	//��Ƶ֡��ȡ�߳�
	class Hisi_DecodeChannel :public DecodeChannel
	{
	public:
		/**
		* ���캯��
		* @param channelIndex ͨ�����
		* @param loginId �����½id
		* @param encodeChannel �����߳�
		*/
		Hisi_DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel);

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

	};

}

