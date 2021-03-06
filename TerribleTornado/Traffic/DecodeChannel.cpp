#include "DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;

const int DecodeChannel::ConnectSpan = 5000;
const int DecodeChannel::GbSleepSpan = 1000;
const int DecodeChannel::MaxHandleSpan = 1000;
const int DecodeChannel::DestinationWidth = 1920;
const int DecodeChannel::DestinationHeight = 1080;

DecodeChannel::DecodeChannel(int channelIndex, int loginId, EncodeChannel* encodeChannel)
	:ThreadObject("decode")
	, _channelIndex(channelIndex), _loginId(loginId)
	, _inputUrl(), _outputUrl(), _channelType(ChannelType::None), _channelStatus(ChannelStatus::Init), _taskId(0), _loop(false), _syncDetect(false)
	, _inputFormat(NULL), _inputStream(NULL), _inputVideoIndex(-1), _sourceWidth(0), _sourceHeight(0), _frameSpan(0), _frameIndex(1), _totalFrameCount(0), _handleFrameCount(0), _currentTaskId(0), _playHandler(-1)
	, _outputHandler()
	, _encodeChannel(encodeChannel)
{
}

void DecodeChannel::InitFFmpeg()
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
	LogPool::Information(LogEvent::Decode, "初始化 ffmpeg");
}

void DecodeChannel::UninitFFmpeg()
{
	avformat_network_deinit();
	LogPool::Information(LogEvent::Decode, "uninit ffmpeg sdk");
}

bool DecodeChannel::InitHisi(int videoCount,int blockCount)
{
#ifndef _WIN32
	HI_S32 hi_s32_ret = HI_SUCCESS;
	SIZE_S stDispSize;
	stDispSize.u32Width = DestinationWidth;
	stDispSize.u32Height = DestinationHeight;
	VB_CONFIG_S stVbConfig;
	memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
	stVbConfig.u32MaxPoolCnt = 1;
	stVbConfig.astCommPool[0].u32BlkCnt = blockCount; //6*u32VdecChnNum;
	stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
		PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 0);
	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();

	hi_s32_ret = HI_MPI_VB_SetConfig(&stVbConfig);
	if (HI_SUCCESS != hi_s32_ret)
	{
		LogPool::Error(LogEvent::Decode, "HI_MPI_VB_SetConfig", StringEx::ToHex(hi_s32_ret));
		return false;
	}

	hi_s32_ret = HI_MPI_VB_Init();

	if (HI_SUCCESS != hi_s32_ret)
	{
		LogPool::Error(LogEvent::Decode, "HI_MPI_VB_Init", StringEx::ToHex(hi_s32_ret));
		return false;
	}

	hi_s32_ret = HI_MPI_SYS_Init();

	if (HI_SUCCESS != hi_s32_ret)
	{
		LogPool::Error(LogEvent::Decode, "HI_MPI_SYS_Init", StringEx::ToHex(hi_s32_ret));

		return false;
	}

	//vedc
	typedef struct hiSAMPLE_VDEC_VIDEO_ATTR
	{
		VIDEO_DEC_MODE_E enDecMode;
		HI_U32              u32RefFrameNum;
		DATA_BITWIDTH_E  enBitWidth;
	}SAMPLE_VDEC_VIDEO_ATTR;

	typedef struct hiSAMPLE_VDEC_PICTURE_ATTR
	{
		PIXEL_FORMAT_E enPixelFormat;
		HI_U32         u32Alpha;
	}SAMPLE_VDEC_PICTURE_ATTR;
	typedef struct hiSAMPLE_VDEC_ATTR
	{
		PAYLOAD_TYPE_E enType;
		VIDEO_MODE_E   enMode;
		HI_U32 u32Width;
		HI_U32 u32Height;
		HI_U32 u32FrameBufCnt;
		HI_U32 u32DisplayFrameNum;
		union
		{
			SAMPLE_VDEC_VIDEO_ATTR stSapmleVdecVideo;      /* structure with video ( h265/h264) */
			SAMPLE_VDEC_PICTURE_ATTR stSapmleVdecPicture; /* structure with picture (jpeg/mjpeg )*/
		};
	}SAMPLE_VDEC_ATTR;
	typedef struct hiSAMPLE_VDEC_BUF
	{
		HI_U32  u32PicBufSzie;
		HI_U32  u32TmvBufSzie;
		HI_BOOL bPicBufAlloc;
		HI_BOOL bTmvBufAlloc;
	}SAMPLE_VDEC_BUF;
	static vector<SAMPLE_VDEC_ATTR> pastSampleVdec;

	pastSampleVdec.resize(videoCount);
	for (int i = 0; i < videoCount; ++i) {
		pastSampleVdec[i].enType = PT_H264;
		pastSampleVdec[i].u32Width = 4096;
		pastSampleVdec[i].u32Height = 2160;
		pastSampleVdec[i].enMode = VIDEO_MODE_FRAME;
		pastSampleVdec[i].stSapmleVdecVideo.enDecMode = VIDEO_DEC_MODE_IPB;
		pastSampleVdec[i].stSapmleVdecVideo.enBitWidth = DATA_BITWIDTH_8;
		pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum = 5;
		pastSampleVdec[i].u32DisplayFrameNum = 5;
		pastSampleVdec[i].u32FrameBufCnt = pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum + pastSampleVdec[i].u32DisplayFrameNum + 1;
	}

	VB_CONFIG_S stVbConf;
	HI_S32 pos = 0;
	HI_BOOL bFindFlag;
	SAMPLE_VDEC_BUF astSampleVdecBuf[VDEC_MAX_CHN_NUM];

	memset(astSampleVdecBuf, 0, sizeof(SAMPLE_VDEC_BUF) * VDEC_MAX_CHN_NUM);
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	for (int i = 0; i < videoCount; i++)
	{
		if (PT_H264 == pastSampleVdec[i].enType)
		{
			astSampleVdecBuf[i].u32PicBufSzie = VDEC_GetPicBufferSize(pastSampleVdec[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height,
				PIXEL_FORMAT_YVU_SEMIPLANAR_420, pastSampleVdec[i].stSapmleVdecVideo.enBitWidth, 0);
			if (VIDEO_DEC_MODE_IPB == pastSampleVdec[i].stSapmleVdecVideo.enDecMode)
			{
				astSampleVdecBuf[i].u32TmvBufSzie = VDEC_GetTmvBufferSize(pastSampleVdec[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height);
			}
		}
	}

	/* PicBuffer */
	for (int j = 0; j < VB_MAX_COMM_POOLS; j++)
	{
		bFindFlag = HI_FALSE;
		for (int i = 0; i < videoCount; i++)
		{
			if ((HI_FALSE == bFindFlag) && (0 != astSampleVdecBuf[i].u32PicBufSzie) && (HI_FALSE == astSampleVdecBuf[i].bPicBufAlloc))
			{
				stVbConf.astCommPool[j].u64BlkSize = astSampleVdecBuf[i].u32PicBufSzie;
				stVbConf.astCommPool[j].u32BlkCnt = pastSampleVdec[i].u32FrameBufCnt;
				astSampleVdecBuf[i].bPicBufAlloc = HI_TRUE;
				bFindFlag = HI_TRUE;
				pos = j;
			}

			if ((HI_TRUE == bFindFlag) && (HI_FALSE == astSampleVdecBuf[i].bPicBufAlloc)
				&& (stVbConf.astCommPool[j].u64BlkSize == astSampleVdecBuf[i].u32PicBufSzie))
			{
				stVbConf.astCommPool[j].u32BlkCnt += pastSampleVdec[i].u32FrameBufCnt;
				astSampleVdecBuf[i].bPicBufAlloc = HI_TRUE;
			}
		}
	}

	/* TmvBuffer */
	for (int j = pos + 1; j < VB_MAX_COMM_POOLS; j++)
	{
		bFindFlag = HI_FALSE;
		for (int i = 0; i < videoCount; i++)
		{
			if ((HI_FALSE == bFindFlag) && (0 != astSampleVdecBuf[i].u32TmvBufSzie) && (HI_FALSE == astSampleVdecBuf[i].bTmvBufAlloc))
			{
				stVbConf.astCommPool[j].u64BlkSize = astSampleVdecBuf[i].u32TmvBufSzie;
				stVbConf.astCommPool[j].u32BlkCnt = pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum + 1;
				astSampleVdecBuf[i].bTmvBufAlloc = HI_TRUE;
				bFindFlag = HI_TRUE;
				pos = j;
			}

			if ((HI_TRUE == bFindFlag) && (HI_FALSE == astSampleVdecBuf[i].bTmvBufAlloc)
				&& (stVbConf.astCommPool[j].u64BlkSize == astSampleVdecBuf[i].u32TmvBufSzie))
			{
				stVbConf.astCommPool[j].u32BlkCnt += pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum + 1;
				astSampleVdecBuf[i].bTmvBufAlloc = HI_TRUE;
			}
		}
	}
	stVbConf.u32MaxPoolCnt = pos + 1;

	hi_s32_ret = HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Error(LogEvent::Decode, "HI_MPI_VB_ExitModCommPool", StringEx::ToHex(hi_s32_ret));
		return false;
	}
	hi_s32_ret = HI_MPI_VB_SetModPoolConfig(VB_UID_VDEC, &stVbConf);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Error(LogEvent::Decode, "HI_MPI_VB_SetModPoolConfig", StringEx::ToHex(hi_s32_ret));
		return false;
	}
	hi_s32_ret = HI_MPI_VB_InitModCommPool(VB_UID_VDEC);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Error(LogEvent::Decode, "HI_MPI_VB_InitModCommPool", StringEx::ToHex(hi_s32_ret));
		HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
		return false;
	}

	VDEC_CHN_ATTR_S stChnAttrs[128];

	VDEC_CHN_PARAM_S stChnParam;
	VDEC_MOD_PARAM_S stModParam;

	hi_s32_ret = HI_MPI_VDEC_GetModParam(&stModParam);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Error(LogEvent::Decode, "HI_MPI_VDEC_GetModParam", StringEx::ToHex(hi_s32_ret));
		return false;
	}
	stModParam.enVdecVBSource = VB_SOURCE_MODULE;
	hi_s32_ret = HI_MPI_VDEC_SetModParam(&stModParam);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Error(LogEvent::Decode, "HI_MPI_VDEC_SetModParam", StringEx::ToHex(hi_s32_ret));
		return false;
	}
	for (int i = 0; i < videoCount; i++)
	{
		stChnAttrs[i].enType = pastSampleVdec[i].enType;
		stChnAttrs[i].enMode = pastSampleVdec[i].enMode;
		stChnAttrs[i].u32PicWidth = pastSampleVdec[i].u32Width;
		stChnAttrs[i].u32PicHeight = pastSampleVdec[i].u32Height;
		stChnAttrs[i].u32StreamBufSize = pastSampleVdec[i].u32Width * pastSampleVdec[i].u32Height;
		stChnAttrs[i].u32FrameBufCnt = pastSampleVdec[i].u32FrameBufCnt;

		if (PT_H264 == pastSampleVdec[i].enType)
		{
			stChnAttrs[i].stVdecVideoAttr.u32RefFrameNum = pastSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum;
			stChnAttrs[i].stVdecVideoAttr.bTemporalMvpEnable = HI_TRUE;
			if ((PT_H264 == pastSampleVdec[i].enType) && (VIDEO_DEC_MODE_IPB != pastSampleVdec[i].stSapmleVdecVideo.enDecMode))
			{
				stChnAttrs[i].stVdecVideoAttr.bTemporalMvpEnable = HI_FALSE;
			}
			stChnAttrs[i].u32FrameBufSize = VDEC_GetPicBufferSize(stChnAttrs[i].enType, pastSampleVdec[i].u32Width, pastSampleVdec[i].u32Height,
				PIXEL_FORMAT_YVU_SEMIPLANAR_420, pastSampleVdec[i].stSapmleVdecVideo.enBitWidth, 0);
		}

		hi_s32_ret = HI_MPI_VDEC_CreateChn(i, &stChnAttrs[i]);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_VDEC_CreateChn", StringEx::ToHex(hi_s32_ret));
			return false;
		}

		hi_s32_ret = HI_MPI_VDEC_GetChnParam(i, &stChnParam);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_VDEC_GetChnParam", StringEx::ToHex(hi_s32_ret));
			return false;
		}
		if (PT_H264 == pastSampleVdec[i].enType)
		{
			stChnParam.stVdecVideoParam.enDecMode = pastSampleVdec[i].stSapmleVdecVideo.enDecMode;
			stChnParam.stVdecVideoParam.enCompressMode = COMPRESS_MODE_TILE;
			stChnParam.stVdecVideoParam.enVideoFormat = VIDEO_FORMAT_TILE_64x16;
			if (VIDEO_DEC_MODE_IPB == stChnParam.stVdecVideoParam.enDecMode)
			{
				stChnParam.stVdecVideoParam.enOutputOrder = VIDEO_OUTPUT_ORDER_DISP;
			}
			else
			{
				stChnParam.stVdecVideoParam.enOutputOrder = VIDEO_OUTPUT_ORDER_DEC;
			}
		}
		else if (PT_JPEG == pastSampleVdec[i].enType)
		{
			stChnParam.stVdecPictureParam.enPixelFormat = pastSampleVdec[i].stSapmleVdecPicture.enPixelFormat;
			stChnParam.stVdecPictureParam.u32Alpha = pastSampleVdec[i].stSapmleVdecPicture.u32Alpha;
		}
		stChnParam.u32DisplayFrameNum = pastSampleVdec[i].u32DisplayFrameNum;
		hi_s32_ret = HI_MPI_VDEC_SetChnParam(i, &stChnParam);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_VDEC_SetChnParam", StringEx::ToHex(hi_s32_ret));
			return false;
		}
		hi_s32_ret = HI_MPI_VDEC_StartRecvStream(i);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_VDEC_StartRecvStream", StringEx::ToHex(hi_s32_ret));
			return false;
		}
	}

	for (int i = 0; i < videoCount; ++i) {
		hi_s32_ret = HI_MPI_VDEC_SetDisplayMode(i, VIDEO_DISPLAY_MODE_PLAYBACK);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_VDEC_SetDisplayMode", StringEx::ToHex(hi_s32_ret));
			return false;
		}
	}

	VPSS_GRP_ATTR_S vpss_grp_attr;
	memset(&vpss_grp_attr, 0, sizeof(VPSS_GRP_ATTR_S));
	vpss_grp_attr.u32MaxW = stDispSize.u32Width;
	vpss_grp_attr.u32MaxH = stDispSize.u32Height;
	vpss_grp_attr.stFrameRate.s32SrcFrameRate = -1;
	vpss_grp_attr.stFrameRate.s32DstFrameRate = -1;
	vpss_grp_attr.enDynamicRange = DYNAMIC_RANGE_SDR8;
	vpss_grp_attr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
	vpss_grp_attr.bNrEn = HI_FALSE;
	vector<HI_BOOL> m_ChnEnableList;
	vector<VPSS_CHN_ATTR_S> vpss_chn_attr_list(8);
	m_ChnEnableList.resize(8);
	m_ChnEnableList[0] = HI_TRUE;
	vpss_chn_attr_list[0].u32Width = stDispSize.u32Width;
	vpss_chn_attr_list[0].u32Height = stDispSize.u32Height;
	vpss_chn_attr_list[0].enChnMode = VPSS_CHN_MODE_USER;
	vpss_chn_attr_list[0].enCompressMode = COMPRESS_MODE_NONE;
	vpss_chn_attr_list[0].enDynamicRange = DYNAMIC_RANGE_SDR8;
	vpss_chn_attr_list[0].enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
	vpss_chn_attr_list[0].stFrameRate.s32SrcFrameRate = -1;
	vpss_chn_attr_list[0].stFrameRate.s32DstFrameRate = -1;
	vpss_chn_attr_list[0].u32Depth = 5;
	vpss_chn_attr_list[0].bMirror = HI_FALSE;
	vpss_chn_attr_list[0].bFlip = HI_FALSE;
	vpss_chn_attr_list[0].stAspectRatio.enMode = ASPECT_RATIO_NONE;
	vpss_chn_attr_list[0].enVideoFormat = VIDEO_FORMAT_LINEAR;
	VPSS_GRP kVpssGrp = 0;
	for (int i = 0; i < videoCount; ++i) {
		kVpssGrp = i;

		VPSS_CHN VpssChn;

		HI_S32 j;

		hi_s32_ret = HI_MPI_VPSS_CreateGrp(kVpssGrp, &vpss_grp_attr);

		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_VPSS_CreateGrp", StringEx::ToHex(hi_s32_ret));
			return false;
		}

		for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++)
		{
			if (HI_TRUE == m_ChnEnableList[j])
			{
				VpssChn = j;
				hi_s32_ret = HI_MPI_VPSS_SetChnAttr(kVpssGrp, VpssChn, &vpss_chn_attr_list[VpssChn]);


				if (hi_s32_ret != HI_SUCCESS)
				{
					LogPool::Error(LogEvent::Decode, "HI_MPI_VPSS_SetChnAttr", StringEx::ToHex(hi_s32_ret));
					return false;
				}


				hi_s32_ret = HI_MPI_VPSS_EnableChn(kVpssGrp, VpssChn);


				if (hi_s32_ret != HI_SUCCESS)
				{
					LogPool::Error(LogEvent::Decode, "HI_MPI_VPSS_EnableChn", StringEx::ToHex(hi_s32_ret));
					return false;
				}

			}
		}

		hi_s32_ret = HI_MPI_VPSS_StartGrp(kVpssGrp);

		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_VPSS_StartGrp", StringEx::ToHex(hi_s32_ret));
			return false;
		}
	}


	for (int i = 0; i < videoCount; ++i) {

		MPP_CHN_S stSrcChn;
		MPP_CHN_S stDestChn;

		stSrcChn.enModId = HI_ID_VDEC;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = i;

		stDestChn.enModId = HI_ID_VPSS;
		stDestChn.s32DevId = i;
		stDestChn.s32ChnId = 0;

		hi_s32_ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Error(LogEvent::Decode, "HI_MPI_SYS_Bind", StringEx::ToHex(hi_s32_ret));
			return false;
		}
	}


	//vo
	//RECT_S                 stDefDispRect = { 0, 0, static_cast<HI_U32>(DestinationWidth), static_cast<HI_U32>(DestinationHeight) };
	//SIZE_S                 stDefImageSize = { static_cast<HI_U32>(DestinationWidth), static_cast<HI_U32>(DestinationHeight) };

	//VO_DEV                 VoDev = 0;
	//VO_LAYER               VoLayer = 0;
	//VO_INTF_TYPE_E         enVoIntfType = VO_INTF_HDMI;

	//VO_PUB_ATTR_S          stVoPubAttr = { 0 };
	//VO_VIDEO_LAYER_ATTR_S  stLayerAttr = { 0 };

	///********************************
	//* Set and start VO device VoDev#.
	//*********************************/
	//stVoPubAttr.enIntfType = VO_INTF_HDMI;
	//stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;

	//stVoPubAttr.u32BgColor = 0x222222;


	//hi_s32_ret = HI_MPI_VO_SetPubAttr(VoDev, &stVoPubAttr);
	//if (hi_s32_ret != HI_SUCCESS)
	//{
	//	LogPool::Error(LogEvent::Decode, "HI_MPI_VO_SetPubAttr", StringEx::ToHex(hi_s32_ret));
	//	return false;
	//}

	//hi_s32_ret = HI_MPI_VO_Enable(VoDev);
	//if (hi_s32_ret != HI_SUCCESS)
	//{
	//	LogPool::Error(LogEvent::Decode, "HI_MPI_VO_Enable", StringEx::ToHex(hi_s32_ret));
	//	return false;
	//}


	///******************************
	//* Set and start layer VoDev#.
	//********************************/
	//stLayerAttr.stDispRect.u32Width = DestinationWidth;
	//stLayerAttr.stDispRect.u32Height = DestinationHeight;
	//stLayerAttr.u32DispFrmRt = 30;
	//stLayerAttr.bClusterMode = HI_FALSE;
	//stLayerAttr.bDoubleFrame = HI_FALSE;
	//stLayerAttr.enPixFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;

	//stLayerAttr.stDispRect.s32X = 0;
	//stLayerAttr.stDispRect.s32Y = 0;

	//stLayerAttr.stDispRect = stDefDispRect;
	//stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
	//stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

	//stLayerAttr.stImageSize = stDefImageSize;
	//stLayerAttr.enDstDynamicRange = DYNAMIC_RANGE_SDR8;


	//hi_s32_ret = HI_MPI_VO_SetDisplayBufLen(VoLayer, 4);
	//if (HI_SUCCESS != hi_s32_ret)
	//{
	//	LogPool::Error(LogEvent::Decode, "HI_MPI_VO_SetDisplayBufLen", StringEx::ToHex(hi_s32_ret));
	//	return false;
	//}


	//hi_s32_ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
	//if (hi_s32_ret != HI_SUCCESS)
	//{
	//	LogPool::Error(LogEvent::Decode, "HI_MPI_VO_SetVideoLayerAttr", StringEx::ToHex(hi_s32_ret));
	//	return false;
	//}

	//hi_s32_ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
	//if (hi_s32_ret != HI_SUCCESS)
	//{
	//	LogPool::Error(LogEvent::Decode, "HI_MPI_VO_EnableVideoLayer", StringEx::ToHex(hi_s32_ret));
	//	return false;
	//}

	///******************************
	//* start vo channels.
	//********************************/
	//hi_s32_ret = HI_SUCCESS;

	//HI_U32 u32Square = 3;
	//HI_U32 u32Width = 0;
	//HI_U32 u32Height = 0;
	//VO_CHN_ATTR_S         stChnAttr;

	//hi_s32_ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	//if (hi_s32_ret != HI_SUCCESS)
	//{
	//	LogPool::Error(LogEvent::Decode, "HI_MPI_VO_GetVideoLayerAttr", StringEx::ToHex(hi_s32_ret));
	//	return false;
	//}
	//u32Width = stLayerAttr.stImageSize.u32Width;
	//u32Height = stLayerAttr.stImageSize.u32Height;


	//for (int i = 0; i < videoCount; i++)
	//{
	//	stChnAttr.stRect.s32X = ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 2);
	//	stChnAttr.stRect.s32Y = ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 2);
	//	stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Square, 2);
	//	stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Square, 2);
	//	stChnAttr.u32Priority = 0;
	//	stChnAttr.bDeflicker = HI_FALSE;

	//	hi_s32_ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
	//	if (hi_s32_ret != HI_SUCCESS)
	//	{
	//		LogPool::Error(LogEvent::Decode, "HI_MPI_VO_SetChnAttr", StringEx::ToHex(hi_s32_ret));
	//		return false;
	//	}

	//	hi_s32_ret = HI_MPI_VO_EnableChn(VoLayer, i);
	//	if (hi_s32_ret != HI_SUCCESS)
	//	{
	//		LogPool::Error(LogEvent::Decode, "HI_MPI_VO_EnableChn", StringEx::ToHex(hi_s32_ret));
	//		return false;
	//	}
	//}

	///******************************
	//* Start hdmi device.
	//* Note : do this after vo device started.
	//********************************/
	//if (VO_INTF_HDMI == enVoIntfType)
	//{
	//	HI_HDMI_ATTR_S          stAttr;
	//	HI_HDMI_VIDEO_FMT_E     enVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
	//	HI_HDMI_ID_E            enHdmiId = HI_HDMI_ID_0;

	//	hi_s32_ret = HI_MPI_HDMI_Init();
	//	if (hi_s32_ret != HI_SUCCESS)
	//	{
	//		LogPool::Error(LogEvent::Decode, "HI_MPI_HDMI_Init", StringEx::ToHex(hi_s32_ret));
	//		return false;
	//	}
	//	hi_s32_ret = HI_MPI_HDMI_Open(enHdmiId);
	//	if (hi_s32_ret != HI_SUCCESS)
	//	{
	//		LogPool::Error(LogEvent::Decode, "HI_MPI_HDMI_Open", StringEx::ToHex(hi_s32_ret));
	//		return false;
	//	}
	//	hi_s32_ret = HI_MPI_HDMI_GetAttr(enHdmiId, &stAttr);
	//	if (hi_s32_ret != HI_SUCCESS)
	//	{
	//		LogPool::Error(LogEvent::Decode, "HI_MPI_HDMI_GetAttr", StringEx::ToHex(hi_s32_ret));
	//		return false;
	//	}
	//	stAttr.bEnableHdmi = HI_TRUE;
	//	stAttr.bEnableVideo = HI_TRUE;
	//	stAttr.enVideoFmt = enVideoFmt;
	//	stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
	//	stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;

	//	stAttr.bxvYCCMode = HI_FALSE;
	//	stAttr.enOutCscQuantization = HDMI_QUANTIZATION_LIMITED_RANGE;

	//	stAttr.bEnableAudio = HI_FALSE;
	//	stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
	//	stAttr.bIsMultiChannel = HI_FALSE;

	//	stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;

	//	stAttr.bEnableAviInfoFrame = HI_TRUE;
	//	stAttr.bEnableAudInfoFrame = HI_TRUE;
	//	stAttr.bEnableSpdInfoFrame = HI_FALSE;
	//	stAttr.bEnableMpegInfoFrame = HI_FALSE;

	//	stAttr.bDebugFlag = HI_FALSE;
	//	stAttr.bHDCPEnable = HI_FALSE;

	//	stAttr.b3DEnable = HI_FALSE;
	//	stAttr.enDefaultMode = HI_HDMI_FORCE_HDMI;
	//	hi_s32_ret = HI_MPI_HDMI_SetAttr(enHdmiId, &stAttr);
	//	if (hi_s32_ret != HI_SUCCESS)
	//	{
	//		LogPool::Error(LogEvent::Decode, "HI_MPI_HDMI_SetAttr", StringEx::ToHex(hi_s32_ret));
	//		return false;
	//	}
	//	hi_s32_ret = HI_MPI_HDMI_Start(enHdmiId);
	//	if (hi_s32_ret != HI_SUCCESS)
	//	{
	//		LogPool::Error(LogEvent::Decode, "HI_MPI_HDMI_Start", StringEx::ToHex(hi_s32_ret));
	//		return false;
	//	}
	//}

#endif
	LogPool::Information(LogEvent::Decode, "初始化海思解码sdk");
	return true;
}

void DecodeChannel::UninitHisi(int videoCount)
{
#ifndef _WIN32
	//vedc
	for (int i = 0; i < videoCount; ++i) {
		MPP_CHN_S stSrcChn;
		MPP_CHN_S stDestChn;

		stSrcChn.enModId = HI_ID_VDEC;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = i;

		stDestChn.enModId = HI_ID_VPSS;
		stDestChn.s32DevId = i;
		stDestChn.s32ChnId = 0;

		HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	}

	VPSS_GRP kVpssGrp = 0;
	for (int i = kVpssGrp; i >= 0; --i) {
		kVpssGrp = i;

		HI_S32 j;
		HI_S32 s32Ret = HI_SUCCESS;
		VPSS_CHN VpssChn;

		s32Ret = HI_MPI_VPSS_StopGrp(kVpssGrp);

		if (s32Ret != HI_SUCCESS)
		{
			return;
		}
		vector<HI_BOOL> m_ChnEnableList;
		vector<VPSS_CHN_ATTR_S> vpss_chn_attr_list(8);
		m_ChnEnableList.resize(8);
		m_ChnEnableList[0] = HI_TRUE;
		for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++)
		{
			if (HI_TRUE == m_ChnEnableList[j])
			{
				VpssChn = j;
				s32Ret = HI_MPI_VPSS_DisableChn(kVpssGrp, VpssChn);

				if (s32Ret != HI_SUCCESS)
				{
					return;
				}
			}
		}

		s32Ret = HI_MPI_VPSS_DestroyGrp(kVpssGrp);

		if (s32Ret != HI_SUCCESS)
		{
			return;
		}
	}

	for (int i = 0; i < videoCount; i++)
	{
		HI_MPI_VDEC_StopRecvStream(i);
		HI_MPI_VDEC_DestroyChn(i);
	}

	HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);

	//vo

	//VO_DEV                VoDev = 0;
	//VO_LAYER              VoLayer = 0;

	//HI_HDMI_ID_E enHdmiId = HI_HDMI_ID_0;

	//HI_MPI_HDMI_Stop(enHdmiId);
	//HI_MPI_HDMI_Close(enHdmiId);
	//HI_MPI_HDMI_DeInit();

	//HI_S32 i;
	//HI_S32 s32Ret = HI_SUCCESS;

	//for (i = 0; i < videoCount; i++)
	//{
	//	s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
	//	if (s32Ret != HI_SUCCESS)
	//	{
	//		return;
	//	}
	//}

	//s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
	//if (s32Ret != HI_SUCCESS)
	//{
	//	return;
	//}

	//s32Ret = HI_MPI_VO_Disable(VoDev);
	//if (s32Ret != HI_SUCCESS)
	//{
	//	return;
	//}
	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();
#endif // !_WIN32
	LogPool::Information(LogEvent::Decode, "卸载海思解码sdk");
}

string DecodeChannel::InputUrl()
{
	lock_guard<mutex> lck(_mutex);
	return _inputUrl;
}

ChannelStatus DecodeChannel::Status()
{
	return _channelStatus;
}

int DecodeChannel::SourceWidth()
{
	return _sourceWidth;
}

int DecodeChannel::SourceHeight()
{
	return _sourceHeight;
}

int DecodeChannel::FrameSpan()
{
	return static_cast<int>(_frameSpan);
}

unsigned char DecodeChannel::UpdateChannel(const TrafficChannel& channel)
{
	lock_guard<mutex> lck(_mutex);
	if (_inputUrl.compare(channel.ChannelUrl) != 0
		|| !channel.Loop
		|| _channelStatus != ChannelStatus::Normal)
	{
		_channelStatus = ChannelStatus::Init;
		++_taskId;
	}
	_channelType = static_cast<ChannelType>(channel.ChannelType);
	_inputUrl.assign(channel.ChannelUrl);
	_outputUrl.assign(channel.RtmpUrl("127.0.0.1"));
	_loop = channel.Loop;
	if (_channelType == ChannelType::File && channel.GlobalDetect)
	{
		_syncDetect = true;
	}
	else
	{
		_syncDetect = false;
	}
	LogPool::Information(LogEvent::Decode, "添加检测通道，通道序号:", channel.ChannelIndex, " 通道地址:", channel.ChannelUrl, " 通道类型：", channel.ChannelType, " 是否循环:", channel.Loop, " 全局检测:", channel.GlobalDetect, " 是否输出图片:", channel.OutputImage);
	return _taskId;
}

void DecodeChannel::ClearChannel()
{
	lock_guard<mutex> lck(_mutex);
	_inputUrl.clear();
	_outputUrl.clear();
	_channelStatus = ChannelStatus::Init;
}

bool DecodeChannel::InitInput(const string& inputUrl)
{
	if (inputUrl.empty())
	{
		return false;
	}
	else
	{
		if (_inputFormat == NULL)
		{
			AVDictionary* options = NULL;
			if (inputUrl.size() >= 4 && inputUrl.substr(0, 4).compare("rtsp") == 0)
			{
				av_dict_set(&options, "rtsp_transport", "tcp", 0);
				av_dict_set(&options, "stimeout", "5000000", 0);
			}
			int result = avformat_open_input(&_inputFormat, inputUrl.c_str(), 0, &options);
			if (result != 0) {
				LogPool::Error(LogEvent::Decode, "avformat_open_input", inputUrl, result);
				UninitInput();
				return false;
			}
			result = avformat_find_stream_info(_inputFormat, NULL);
			if (result < 0) {
				LogPool::Error(LogEvent::Decode, "avformat_find_stream_info", inputUrl, result);
				UninitInput();
				return false;
			}

			for (unsigned int i = 0; i < _inputFormat->nb_streams; i++) {
				if (_inputFormat->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
					_inputVideoIndex = i;
					break;
				}
			}
			if (_inputVideoIndex == -1) {
				LogPool::Error(LogEvent::Decode, "not found video index", inputUrl);
				UninitInput();
				return false;
			}
			_inputStream = _inputFormat->streams[_inputVideoIndex];
			_sourceWidth = _inputStream->codecpar->width;
			_sourceHeight = _inputStream->codecpar->height;
			if (_inputStream->avg_frame_rate.den != 0)
			{
				_frameSpan = static_cast<unsigned char>(1000 / (_inputStream->avg_frame_rate.num / _inputStream->avg_frame_rate.den));
			}
			else if (_inputStream->r_frame_rate.den != 0)
			{
				_frameSpan = static_cast<unsigned char>(1000 / (_inputStream->r_frame_rate.num / _inputStream->r_frame_rate.den));
			}
			else
			{
				_frameSpan = 40;
				LogPool::Warning(LogEvent::Decode, "not found ffmpeg frame_rate,use default 40ms,input url:", inputUrl);
			}
		}
		return true;
	}
}

void DecodeChannel::UninitInput()
{
	if (_inputFormat != NULL) {
		_inputVideoIndex = -1;
		_inputStream = NULL;
		avformat_close_input(&_inputFormat);
		_inputFormat = NULL;
		_sourceWidth = 0;
		_sourceHeight = 0;
		_frameSpan = 0;
		LogPool::Information(LogEvent::Decode, "uninit input url:", _inputUrl);
	}
}

void DecodeChannel::ReceivePacket(int playFd, int frameType, char* buffer, unsigned int size, void* usr)
{
	if (frameType == 3)
	{
		return;
	}
	DecodeChannel* channel = static_cast<DecodeChannel*>(usr);
	if (channel->_channelStatus == ChannelStatus::Normal)
	{
		long long timeStamp1 = DateTime::NowTimeStamp();
		channel->_outputHandler.WritePacket(reinterpret_cast<const unsigned char*>(buffer), size, frameType == 0 ? FrameType::I : FrameType::P);
		bool decodeResult = channel->Decode(reinterpret_cast<unsigned char*>(buffer), size, channel->_currentTaskId, channel->_frameIndex, channel->_frameSpan);
		if (channel->_frameIndex % 100 == 0)
		{
			LogPool::Debug(LogEvent::Decode, "got gb data", channel->_frameIndex, static_cast<int>(channel->_channelStatus), static_cast<int>(decodeResult), frameType, size);
		}
		if (!decodeResult)
		{
			LogPool::Error(LogEvent::Decode, "decode error", channel->_channelIndex, channel->_frameIndex);
			channel->_channelStatus = ChannelStatus::DecodeError;
		}
		long long timeStamp2 = DateTime::NowTimeStamp();

		channel->_frameIndex += 1;
		long long sleepTime = channel->_frameSpan - (timeStamp2 - timeStamp1);
		if (sleepTime > 0 && sleepTime <= channel->_frameSpan)
		{
			this_thread::sleep_for(chrono::milliseconds(sleepTime));
		}
	}

}

void DecodeChannel::StartCore()
{
	AVPacket* packet = av_packet_alloc();
	AVPacket* tempPacket = av_packet_alloc();
	bool reportFinish = false;
	string inputUrl;
	ChannelType channelType = ChannelType::None;
	AVBitStreamFilterContext* h264bsfc = NULL;
	while (!_cancelled)
	{
		if (_channelStatus == ChannelStatus::Normal)
		{
			if (channelType == ChannelType::GB28181)
			{
				this_thread::sleep_for(chrono::milliseconds(GbSleepSpan));
			}
			else
			{
				long long timeStamp1 = DateTime::NowTimeStamp();
				int readResult = av_read_frame(_inputFormat, packet);
				if (readResult == AVERROR_EOF)
				{
					if (channelType == ChannelType::File && !_loop)
					{
						LogPool::Information(LogEvent::Decode, "视频文件播放结束，视频序号：", _channelIndex, " 视频地址：", _inputUrl, " 总帧数:", _totalFrameCount, " 处理帧数:", _handleFrameCount);
						_channelStatus = ChannelStatus::ReadEOF_Stop;
					}
					else
					{
						_channelStatus = ChannelStatus::ReadEOF_Restart;
					}
				}
				else if (readResult == 0)
				{
					if (packet->stream_index == _inputVideoIndex)
					{
						_outputHandler.WritePacket(packet);
						int filterResult = av_bitstream_filter_filter(h264bsfc, _inputStream->codec, NULL, &tempPacket->data, &tempPacket->size, packet->data, packet->size, 0);
						if (filterResult < 0)
						{
							LogPool::Error(LogEvent::Decode, "av_bitstream_filter_filter", _channelIndex, _frameIndex, filterResult);
							_channelStatus = ChannelStatus::FilterError;
						}
						else
						{
							long long timeStamp2 = DateTime::NowTimeStamp();
							bool decodeResult = Decode(tempPacket->data, tempPacket->size, _currentTaskId, _frameIndex, _frameSpan);
							if (!decodeResult)
							{
								LogPool::Error(LogEvent::Decode, "decode error,input url:", inputUrl, " frame index", _frameIndex);
								_channelStatus = ChannelStatus::DecodeError;
							}
							long long timeStamp3 = DateTime::NowTimeStamp();
							long long sleepTime = _frameSpan - (timeStamp3 - timeStamp1);
							if (sleepTime > 0 && sleepTime <= _frameSpan)
							{
								this_thread::sleep_for(chrono::milliseconds(sleepTime));
							}
							if (_frameIndex % 100 == 0)
							{
								LogPool::Debug(LogEvent::Decode, "frame->channel index:", _channelIndex, "packet size:", packet->size, "task id:", static_cast<int>(_currentTaskId), "frame index:", _frameIndex, "frame span:", static_cast<int>(_frameSpan), "decode result:", static_cast<int>(decodeResult), "read and write:", timeStamp2 - timeStamp1, "decode:", timeStamp3 - timeStamp2);
							}
							if (filterResult > 0)
							{
								av_free(tempPacket->data);
							}
							_frameIndex += 1;
						}
					}
				}
				else
				{
					LogPool::Error(LogEvent::Decode, "av_read_frame,input url:", inputUrl, " frame index:", _frameIndex, "返回结果:", readResult);
					_channelStatus = ChannelStatus::ReadError;
				}
				av_packet_unref(packet);
			}
		}
		else
		{
			//播放文件结束时报一次结束
			if (_channelStatus == ChannelStatus::ReadEOF_Stop && !reportFinish)
			{
				Decode(NULL, 0, _currentTaskId, _frameIndex, _frameSpan);
				reportFinish = true;
			}
			unique_lock<mutex> lck(_mutex);
			channelType = _channelType;
			if (h264bsfc != NULL)
			{
				av_bitstream_filter_close(h264bsfc);
				h264bsfc = NULL;
			}
			//读取到结尾重新启动时不需要重置输出
			if (_channelStatus != ChannelStatus::ReadEOF_Restart)
			{
				_outputHandler.Uninit();
			}
			if (channelType == ChannelType::GB28181)
			{
#ifndef _WIN32	
				if (_playHandler >= 0)
				{
					int result = vas_sdk_stop_realplay(_playHandler);
					LogPool::Information(LogEvent::Decode, "stop gb,input url:", _inputUrl, ",play handler:", _playHandler, ",result:", result);
					_playHandler = -1;
				}
#endif // !_WIN32
			}
			else
			{
				UninitInput();
			}
			//循环停止和解码错误时不再重新打开视频
			if (_channelStatus != ChannelStatus::ReadEOF_Stop
				&& _channelStatus != ChannelStatus::NotHandle
				&& _channelStatus != ChannelStatus::DecodeError)
			{
				//防止局部变量改动影响前端效果,所以现用临时变量存放,最后确定状态在赋值
				ChannelStatus tempStatus;
				//输入
				if (channelType == ChannelType::GB28181)
				{
#ifdef _WIN32				
					tempStatus = ChannelStatus::InputError;
#else					
					if (_inputUrl.empty())
					{
						tempStatus = ChannelStatus::Init;
					}
					else if (_loginId < 0)
					{
						tempStatus = ChannelStatus::InputError;
					}
					else
					{
						_playHandler = vas_sdk_realplay(_loginId, const_cast<char*>(_inputUrl.c_str()), ReceivePacket, this);
						if (_playHandler >= 0)
						{
							tempStatus = ChannelStatus::Normal;
							LogPool::Information(LogEvent::Decode, "open gb,input url:", _inputUrl, ",play hanlder:", _playHandler);
						}
						else
						{
							tempStatus = ChannelStatus::Init;
							LogPool::Error(LogEvent::Decode, "oopen gb failed,input url:", _inputUrl, ",play handler:", _playHandler);
						}
					}
#endif // !_WIN32
				}
				else
				{
					if (_inputUrl.empty())
					{
						tempStatus = ChannelStatus::Init;
					}
					else
					{
						tempStatus = InitInput(_inputUrl) ? ChannelStatus::Normal : ChannelStatus::InputError;
					}
				}

				//输出
				if (_channelStatus != ChannelStatus::ReadEOF_Restart
					&& tempStatus == ChannelStatus::Normal)
				{
					if (_channelType == ChannelType::GB28181)
					{
						tempStatus = _outputHandler.Init(_outputUrl, -1)
							? ChannelStatus::Normal
							: ChannelStatus::OutputError;
					}
					else
					{
						tempStatus = _outputHandler.Init(_outputUrl, -1, _inputStream->codecpar)
							? ChannelStatus::Normal
							: ChannelStatus::OutputError;
					}
				}

				_channelStatus = tempStatus;
			}

			if (_channelStatus == ChannelStatus::Normal)
			{
				h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
				_frameIndex = 1;
				_totalFrameCount = 0;
				_handleFrameCount = 0;
				_currentTaskId = _taskId;
				reportFinish = false;
				inputUrl = _inputUrl;
				LogPool::Debug(LogEvent::Decode, "init decode channel, input url:", _inputUrl, ",output url:", _outputUrl, ",frame span(ms):", static_cast<int>(_frameSpan), ",loop:", _loop);
				lck.unlock();
			}
			else
			{
				lck.unlock();
				this_thread::sleep_for(chrono::milliseconds(ConnectSpan));
			}
		}
	}
	av_bitstream_filter_close(h264bsfc);
	av_packet_free(&tempPacket);
	av_packet_free(&packet);
	UninitInput();
	_outputHandler.Uninit();
}

