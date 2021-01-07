#pragma once
#include "DecodeChannel.h"
#include "DG_FrameHandler.h"

namespace OnePunchMan
{
	//��Ƶ֡��ȡ�߳�
	class DG_DecodeChannel :public DecodeChannel
	{
	public:
		/**
		* ���캯��
		* @param channelIndex ͨ�����
		* @param loginId �����½id
		* @param encodeChannel �����߳�
		* @param frameHandler ֡����
		*/
		DG_DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel, DG_FrameHandler* frameHandler);

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
		//����֡
		DG_FrameHandler* _frameHandler;
	};

}

