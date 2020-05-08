#pragma once
#include "FFmpegChannel.h"
#include "DetectChannel.h"

extern "C"
{
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#ifndef _WIN32
#include "hi_common.h"
#include "hi_buffer.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vpss.h"
#include "hi_comm_avs.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"
#include "hi_comm_hdmi.h"
#include "hi_mipi.h"
#include "hi_comm_hdr.h"
#include "hi_comm_vgs.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vdec.h"
#include "mpi_vpss.h"
#include "mpi_avs.h"
#include "mpi_region.h"
#include "mpi_audio.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "hi_math.h"
#include "hi_sns_ctrl.h"
#include "mpi_hdmi.h"
#include "mpi_hdr.h"
#include "mpi_vgs.h"
#include "mpi_ive.h"
#endif // !_WIN32

namespace OnePunchMan
{
	//�����߳�
	class DecodeChannel :public FFmpegChannel
	{
	public:
		/**
		* @brief: ���캯��
		* @param: inputUrl ������Ƶ
		* @param: outputUrl �����Ƶ
		* @param: channelIndex ͨ�����
		* @param: detectChannel ����߳�
		* @param: debug �Ƿ��ڵ���ģʽ,���ڵ���ģʽͬ�������㷨
		*/
		DecodeChannel(const std::string& inputUrl,const std::string& outputUrl,int channelIndex, DetectChannel* detectChannel, bool debug=false);
		
		/**
		* @brief: ��ʼ��hisi sdk
		* @param: videoCount ͨ������
		* @return: ��ʼ���ɹ�����true�����򷵻�false
		*/
		static bool InitHisi(int videoCount);

		/**
		* @brief: ж��hisi sdk
		* @param: videoCount ͨ������
		*/
		static void UninitHisi(int videoCount);

	protected:
		bool InitDecoder();

		void UninitDecoder();

		DecodeResult Decode(const AVPacket* packet, int packetIndex);

	private:

		/**
		* @brief: ʹ��hisi����
		* @param: packet ��Ƶ֡
		* @param: packetIndex ��Ƶ֡���
		* @return: ����ɹ�����Handle,�Թ�����Skip,���򷵻�Error
		*/
		DecodeResult DecodeByHisi(const AVPacket* packet, int packetIndex);
		
		/**
		* @brief: ʹ��ffmpeg����
		* @param: packet ��Ƶ֡
		* @param: packetIndex ��Ƶ֡���
		* @return: ����ɹ�����Handle,�Թ�����Skip,���򷵻�Error
		*/
		DecodeResult DecodeByFFmpeg(const AVPacket* packet, int packetIndex);

		//ͨ�����
		int _channelIndex;

		//�Ƿ�ʹ��ffmpeg���룬����ʹ��hisi
		bool _useFFmpeg;

		//ffmpeg�������
		AVCodecContext* _decodeContext;
		//������yuv����
		AVFrame* _yuvFrame;
		//yuv420sp�����ֽ���
		uint8_t* _yuv420spBuffer;
		//yuv420sp�����ֽ�������
		int _yuv420spSize;
		//yuv420sp֡����
		AVFrame* _yuv420spFrame;
		//yuv420spת��
		SwsContext* _yuv420spSwsContext;
		//����߳�
		DetectChannel* _detectChannel;
	};

}

