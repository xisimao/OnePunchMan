#pragma once
#include "FrameChannel.h"
#include "YUV420SPHandler.h"
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


namespace TerribleTornado
{
	class HisiChannel :public Fubuki::FrameChannel
	{
	public:
		HisiChannel(int channelIndex, DetectChannel* detectChannel);

		static int InitHisi(int videoCount);
		static void UninitHisi(int videoCount);

		static const int VideoWidth;
		static const int VideoHeight;
		static const int YUVSize;
	protected:
		int InitCore();

		void UninitCore();

		bool Decode(const AVPacket* packet, int packetIndex);

	private:

		bool DecodeByHisi(const AVPacket* packet, int packetIndex);

		bool DecodeByFFmpeg(const AVPacket* packet, int packetIndex);

		int _channelIndex;
		bool _downgrade;

		AVCodecContext* _decodeContext;
		AVFrame* _yuvFrame;
		uint8_t* _yuv420spBuffer;
		AVFrame* _yuv420spFrame;
		SwsContext* _yuv420spSwsContext;

		Fubuki::YUV420SPHandler* _yuvHandler;
		DetectChannel* _detectChannel;
		int _detectIndex;
	
	};

}

