#include "HisiChannel.h"

using namespace std;
using namespace Saitama;
using namespace Fubuki;
using namespace TerribleTornado;

const int HisiChannel::VideoWidth = 1920;
const int HisiChannel::VideoHeight = 1080;
const int HisiChannel::YUVSize = 1920*1080*1.5;

HisiChannel::HisiChannel(int channelIndex,DetectChannel* detectChannel)
	:FrameChannel(),_channelIndex(channelIndex), _downgrade(false), _decodeContext(NULL), _yuvFrame(NULL), _yuv420spBuffer(NULL), _yuv420spFrame(NULL), _yuv420spSwsContext(NULL),_detectChannel(detectChannel),_detectIndex(channelIndex% DetectChannel::ItemCount)
{
	_yuvHandler = new YUV420SPHandler();
}

int HisiChannel::InitHisi(int videoCount)
{
#ifndef _WIN32
	HI_S32 hi_s32_ret = HI_SUCCESS;
	SIZE_S stDispSize;
	stDispSize.u32Width = VideoWidth;
	stDispSize.u32Height = VideoHeight;
	VB_CONFIG_S stVbConfig;
	memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
	stVbConfig.u32MaxPoolCnt = 1;
	stVbConfig.astCommPool[0].u32BlkCnt = 64; //6*u32VdecChnNum;
	stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
		PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 0);
	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();

	hi_s32_ret = HI_MPI_VB_SetConfig(&stVbConfig);
	if (HI_SUCCESS != hi_s32_ret)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VB_SetConfig", hi_s32_ret);
		return HI_FAILURE;
	}

	hi_s32_ret = HI_MPI_VB_Init();

	if (HI_SUCCESS != hi_s32_ret)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VB_Init", hi_s32_ret);
		return HI_FAILURE;
	}

	hi_s32_ret = HI_MPI_SYS_Init();

	if (HI_SUCCESS != hi_s32_ret)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_SYS_Init", hi_s32_ret);

		return HI_FAILURE;
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
		pastSampleVdec[i].u32Width = 3072;
		pastSampleVdec[i].u32Height = 2210;
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
		LogPool::Information(LogEvent::Decode, "HI_MPI_VB_ExitModCommPool", hi_s32_ret);
		return HI_FAILURE;
	}
	hi_s32_ret = HI_MPI_VB_SetModPoolConfig(VB_UID_VDEC, &stVbConf);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VB_SetModPoolConfig", hi_s32_ret);
		return HI_FAILURE;
	}
	hi_s32_ret = HI_MPI_VB_InitModCommPool(VB_UID_VDEC);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VB_SetModPoolConfig", hi_s32_ret);
		HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
		return HI_FAILURE;
	}

	VDEC_CHN_ATTR_S stChnAttrs[128];

	VDEC_CHN_PARAM_S stChnParam;
	VDEC_MOD_PARAM_S stModParam;

	hi_s32_ret = HI_MPI_VDEC_GetModParam(&stModParam);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VDEC_GetModParam", hi_s32_ret);
		return HI_FAILURE;
	}
	stModParam.enVdecVBSource = VB_SOURCE_MODULE;
	hi_s32_ret = HI_MPI_VDEC_SetModParam(&stModParam);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VDEC_SetModParam", hi_s32_ret);
		return HI_FAILURE;
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
			LogPool::Information(LogEvent::Decode, "HI_MPI_VDEC_CreateChn", hi_s32_ret);
			return HI_FAILURE;
		}

		hi_s32_ret = HI_MPI_VDEC_GetChnParam(i, &stChnParam);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_VDEC_GetChnParam", hi_s32_ret);
			return HI_FAILURE;
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
			LogPool::Information(LogEvent::Decode, "HI_MPI_VDEC_SetChnParam", hi_s32_ret);
			return HI_FAILURE;
		}
		hi_s32_ret = HI_MPI_VDEC_StartRecvStream(i);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_VDEC_StartRecvStream", hi_s32_ret);
			return HI_FAILURE;
		}
	}

	for (int i = 0; i < videoCount; ++i) {
		hi_s32_ret = HI_MPI_VDEC_SetDisplayMode(i, VIDEO_DISPLAY_MODE_PLAYBACK);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_VDEC_SetDisplayMode", hi_s32_ret);
			return HI_FAILURE;
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
			LogPool::Information(LogEvent::Decode, "HI_MPI_VPSS_CreateGrp", hi_s32_ret);
			return HI_FAILURE;
		}

		for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++)
		{
			if (HI_TRUE == m_ChnEnableList[j])
			{
				VpssChn = j;
				hi_s32_ret = HI_MPI_VPSS_SetChnAttr(kVpssGrp, VpssChn, &vpss_chn_attr_list[VpssChn]);


				if (hi_s32_ret != HI_SUCCESS)
				{
					LogPool::Information(LogEvent::Decode, "HI_MPI_VPSS_SetChnAttr", hi_s32_ret);
					return HI_FAILURE;
				}


				hi_s32_ret = HI_MPI_VPSS_EnableChn(kVpssGrp, VpssChn);


				if (hi_s32_ret != HI_SUCCESS)
				{
					LogPool::Information(LogEvent::Decode, "HI_MPI_VPSS_EnableChn", hi_s32_ret);
					return HI_FAILURE;
				}

			}
		}

		hi_s32_ret = HI_MPI_VPSS_StartGrp(kVpssGrp);

		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_VPSS_StartGrp", hi_s32_ret);
			return HI_FAILURE;
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
			LogPool::Information(LogEvent::Decode, "HI_MPI_SYS_Bind", hi_s32_ret);
			return HI_FAILURE;
		}
	}


	//vo
	RECT_S                 stDefDispRect = { 0, 0, VideoWidth, VideoHeight };
	SIZE_S                 stDefImageSize = { VideoWidth, VideoHeight };

	VO_DEV                 VoDev = 0;
	VO_LAYER               VoLayer = 0;
	VO_INTF_TYPE_E         enVoIntfType = VO_INTF_HDMI;

	VO_PUB_ATTR_S          stVoPubAttr = { 0 };
	VO_VIDEO_LAYER_ATTR_S  stLayerAttr = { 0 };

	/********************************
	* Set and start VO device VoDev#.
	*********************************/
	stVoPubAttr.enIntfType = VO_INTF_HDMI;
	stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;

	stVoPubAttr.u32BgColor = 0x222222;


	hi_s32_ret = HI_MPI_VO_SetPubAttr(VoDev, &stVoPubAttr);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VO_SetPubAttr", hi_s32_ret);
		return HI_FAILURE;
	}

	hi_s32_ret = HI_MPI_VO_Enable(VoDev);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VO_Enable", hi_s32_ret);
		return HI_FAILURE;
	}


	/******************************
	* Set and start layer VoDev#.
	********************************/
	stLayerAttr.stDispRect.u32Width = VideoWidth;
	stLayerAttr.stDispRect.u32Height = VideoHeight;
	stLayerAttr.u32DispFrmRt = 30;
	stLayerAttr.bClusterMode = HI_FALSE;
	stLayerAttr.bDoubleFrame = HI_FALSE;
	stLayerAttr.enPixFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;

	stLayerAttr.stDispRect.s32X = 0;
	stLayerAttr.stDispRect.s32Y = 0;

	stLayerAttr.stDispRect = stDefDispRect;
	stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
	stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

	stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
	stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;


	stLayerAttr.stImageSize = stDefImageSize;
	stLayerAttr.enDstDynamicRange = DYNAMIC_RANGE_SDR8;


	hi_s32_ret = HI_MPI_VO_SetDisplayBufLen(VoLayer, 4);
	if (HI_SUCCESS != hi_s32_ret)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VO_SetDisplayBufLen", hi_s32_ret);
		return hi_s32_ret;
	}


	hi_s32_ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VO_SetVideoLayerAttr", hi_s32_ret);
		return HI_FAILURE;
	}

	hi_s32_ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VO_EnableVideoLayer", hi_s32_ret);
		return HI_FAILURE;
	}

	/******************************
	* start vo channels.
	********************************/
	hi_s32_ret = HI_SUCCESS;

	HI_U32 u32Square = 3;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	VO_CHN_ATTR_S         stChnAttr;

	hi_s32_ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (hi_s32_ret != HI_SUCCESS)
	{
		LogPool::Information(LogEvent::Decode, "HI_MPI_VO_GetVideoLayerAttr", hi_s32_ret);
		return HI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;


	for (int i = 0; i < videoCount; i++)
	{
		stChnAttr.stRect.s32X = ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 2);
		stChnAttr.stRect.s32Y = ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 2);
		stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Square, 2);
		stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Square, 2);
		stChnAttr.u32Priority = 0;
		stChnAttr.bDeflicker = HI_FALSE;

		hi_s32_ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_VO_SetChnAttr", hi_s32_ret);
			return HI_FAILURE;
		}

		hi_s32_ret = HI_MPI_VO_EnableChn(VoLayer, i);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_VO_EnableChn", hi_s32_ret);
			return HI_FAILURE;
		}
	}

	/******************************
	* Start hdmi device.
	* Note : do this after vo device started.
	********************************/
	if (VO_INTF_HDMI == enVoIntfType)
	{
		HI_HDMI_ATTR_S          stAttr;
		HI_HDMI_VIDEO_FMT_E     enVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
		HI_HDMI_ID_E            enHdmiId = HI_HDMI_ID_0;

		hi_s32_ret = HI_MPI_HDMI_Init();
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_HDMI_Init", hi_s32_ret);
			return HI_FAILURE;
		}
		hi_s32_ret = HI_MPI_HDMI_Open(enHdmiId);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_HDMI_Open", hi_s32_ret);
			return HI_FAILURE;
		}
		hi_s32_ret = HI_MPI_HDMI_GetAttr(enHdmiId, &stAttr);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_HDMI_GetAttr", hi_s32_ret);
			return HI_FAILURE;
		}
		stAttr.bEnableHdmi = HI_TRUE;
		stAttr.bEnableVideo = HI_TRUE;
		stAttr.enVideoFmt = enVideoFmt;
		stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
		stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;

		stAttr.bxvYCCMode = HI_FALSE;
		stAttr.enOutCscQuantization = HDMI_QUANTIZATION_LIMITED_RANGE;

		stAttr.bEnableAudio = HI_FALSE;
		stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
		stAttr.bIsMultiChannel = HI_FALSE;

		stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;

		stAttr.bEnableAviInfoFrame = HI_TRUE;
		stAttr.bEnableAudInfoFrame = HI_TRUE;
		stAttr.bEnableSpdInfoFrame = HI_FALSE;
		stAttr.bEnableMpegInfoFrame = HI_FALSE;

		stAttr.bDebugFlag = HI_FALSE;
		stAttr.bHDCPEnable = HI_FALSE;

		stAttr.b3DEnable = HI_FALSE;
		stAttr.enDefaultMode = HI_HDMI_FORCE_HDMI;
		hi_s32_ret = HI_MPI_HDMI_SetAttr(enHdmiId, &stAttr);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_HDMI_SetAttr", hi_s32_ret);
			return HI_FAILURE;
		}
		hi_s32_ret = HI_MPI_HDMI_Start(enHdmiId);
		if (hi_s32_ret != HI_SUCCESS)
		{
			LogPool::Information(LogEvent::Decode, "HI_MPI_HDMI_Start", hi_s32_ret);
			return HI_FAILURE;
		}
	}

	////venc
	//int ret = 0;
	//VENC_RECV_PIC_PARAM_S stRecvParam;
	//VENC_GOP_MODE_E enGopMode;
	//VENC_GOP_ATTR_S stGopAttr;
	//SAMPLE_RC_E     enRcMode;
	//HI_S32 s32VencChn[8] = { 0,1,2,3,4,5,6,7 };

	//std::memset(&stGopAttr, 0, sizeof(VENC_GOP_ATTR_S));
	//enGopMode = VENC_GOPMODE_NORMALP;
	//enRcMode = 0;
	//stRecvParam.s32RecvPicNum = -1;


	//HI_S32 hi_s32_ret = HI_SUCCESS;
	//VO_INTF_SYNC_E VoIntfSync;// = VO_OUTPUT_1080P60;
	//PIC_SIZE_E disp_pic_size;
	//switch (VoIntfSync) {
	//case VO_OUTPUT_3840x2160_30:
	//	disp_pic_size = PIC_3840x2160;
	//	break;
	//case VO_OUTPUT_1080P60:
	//	disp_pic_size = PIC_1080P;
	//	break;
	//default:
	//	disp_pic_size = PIC_1080P;
	//	break;
	//}
	//stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
	//stGopAttr.stNormalP.s32IPQpDelta = 3;
	////ret = SAMPLE_COMM_VENC_GetGopAttr(enGopMode, &stGopAttr);
	////if (ret != 0)
	////{
	////	LOG(ERROR) << "SAMPLE_COMM_VENC_GetGopAttr faild!";
	////}
	//for (int i = 0; i < 8; i++)
	//{
	//	ret = SEEMMO_COMM_VENC_Start(s32VencChn[i], PT_H264, disp_pic_size, enRcMode, 0, &stGopAttr);
	//	if (ret != 0)
	//	{
	//		LOG(ERROR) << "SEEMMO_COMM_VENC_Start faild!";
	//	}
	//}

	//ret = SEEMMO_COMM_VENC_StartGetStream(s32VencChn, 8);
	//if (ret != 0)
	//{
	//	LOG(ERROR) << "SEEMMO_COMM_VENC_StartGetStream faild!";
	//}

	//RAW_LOG(INFO, "Venc_Init success!\n");
	//return HI_SUCCESS;
#endif
	return 0;
}

void HisiChannel::UninitHisi(int videoCount)
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

	VO_DEV                VoDev = 0;
	VO_LAYER              VoLayer = 0;

	HI_HDMI_ID_E enHdmiId = HI_HDMI_ID_0;

	HI_MPI_HDMI_Stop(enHdmiId);
	HI_MPI_HDMI_Close(enHdmiId);
	HI_MPI_HDMI_DeInit();

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;

	for (i = 0; i < videoCount; i++)
	{
		s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
		if (s32Ret != HI_SUCCESS)
		{
			return;
		}
	}

	s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
	if (s32Ret != HI_SUCCESS)
	{
		return;
	}

	s32Ret = HI_MPI_VO_Disable(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
		return;
	}
	HI_MPI_SYS_Exit();
	HI_MPI_VB_Exit();
#endif // !_WIN32
}

int HisiChannel::InitCore()
{
	if (!_downgrade)
	{
		return 0;
	}
	if (_formatContext->streams[_videoIndex]->codecpar->codec_id != AVCodecID::AV_CODEC_ID_H264)
	{
		LogPool::Error(LogEvent::Decode, "codec is not h264", _url);
		return 4;
	}

	AVCodec* decode = avcodec_find_decoder(_formatContext->streams[_videoIndex]->codecpar->codec_id);
	if (decode == NULL) {
		LogPool::Error(LogEvent::Decode, "avcodec_find_decoder error", _url);
		return 5;
	}
	_decodeContext = avcodec_alloc_context3(decode);
	if (avcodec_parameters_to_context(_decodeContext, _formatContext->streams[_videoIndex]->codecpar) < 0) {
		LogPool::Error(LogEvent::Decode, "avcodec_parameters_to_context error", _url);
		return 6;
	}
	_decodeContext->thread_count = 4;
	_decodeContext->thread_type = FF_THREAD_FRAME;
	if (avcodec_open2(_decodeContext, decode, NULL) < 0) {//打开解码器
		LogPool::Error(LogEvent::Decode, "avcodec_open2 error", _url);
		return 7;
	}

	//初始化帧
	_yuvFrame = av_frame_alloc();

	//yuv转rgb
	_yuv420spFrame = av_frame_alloc();

	_yuv420spBuffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_NV21, VideoWidth, VideoHeight, 1) * sizeof(uint8_t));
	if (av_image_fill_arrays(_yuv420spFrame->data, _yuv420spFrame->linesize, _yuv420spBuffer, AV_PIX_FMT_NV21, VideoWidth, VideoHeight, 1) < 0)
	{
		LogPool::Error(LogEvent::Decode, "av_image_fill_arrays error");
		return 8;
	}
	_yuv420spSwsContext = sws_getContext(_decodeContext->width, _decodeContext->height, _decodeContext->pix_fmt, VideoWidth, VideoHeight, AV_PIX_FMT_NV21,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if (_yuv420spSwsContext == NULL)
	{
		LogPool::Error(LogEvent::Decode, "sws_getContext error", _url);
		return 9;
	}
	return 0;
}

void HisiChannel::UninitCore()
{
	if (_yuv420spSwsContext != NULL)
	{
		sws_freeContext(_yuv420spSwsContext);
	}
	if (_decodeContext != NULL) {
		avcodec_free_context(&_decodeContext);
	}
	if (_yuv420spFrame != NULL)
	{
		av_frame_free(&_yuv420spFrame);
	}
	if (_yuvFrame != NULL)
	{
		av_frame_free(&_yuvFrame);
	}
}

bool HisiChannel::Decode(const AVPacket* packet, int packetIndex)
{
	if (_downgrade)
	{
		return DecodeByFFmpeg(packet, packetIndex);
	}
	else
	{
		if (!DecodeByHisi(packet, packetIndex))
		{
			LogPool::Warning(LogEvent::Decode, "decode downgrade", _url);
			_downgrade = true;
			if (InitCore() != 0)
			{
				return false;
			}
		}
		return true;
	}
}

bool HisiChannel::DecodeByHisi(const AVPacket* packet, int packetIndex)
{
#ifndef _WIN32
	int hi_s32_ret = HI_SUCCESS;
	if (packet != NULL)
	{
		VDEC_STREAM_S stStream;
		stStream.u64PTS = packetIndex;
		stStream.pu8Addr = packet->data;
		stStream.u32Len = packet->size;
		stStream.bEndOfFrame = HI_TRUE;
		stStream.bEndOfStream = HI_FALSE;
		stStream.bDisplay = HI_TRUE;
		hi_s32_ret = HI_MPI_VDEC_SendStream(_channelIndex, &stStream, 0);
		if (HI_SUCCESS != hi_s32_ret) {
			LogPool::Debug("send error", _channelIndex, hi_s32_ret);
			return false;
		}
		//else
		//{
		//	LogPool::Debug("send success", _channelIndex, packetIndex);
		//}
	}
	VIDEO_FRAME_INFO_S frame;
	while (true)
	{
		hi_s32_ret = HI_MPI_VPSS_GetChnFrame(_channelIndex, 0, &frame, 0);
		if (hi_s32_ret == HI_SUCCESS)
		{
			//LogPool::Debug("receive success", _channelIndex, packetIndex);
			if (!_detectChannel->IsBusy(_detectIndex))
			{
				unsigned char* yuv = reinterpret_cast<unsigned char*>(HI_MPI_SYS_Mmap(frame.stVFrame.u64PhyAddr[0], YUVSize));
				_detectChannel->HandleYUV(yuv, VideoWidth, VideoHeight, static_cast<int>(frame.stVFrame.u64PTS), _detectIndex);
				hi_s32_ret = HI_MPI_SYS_Munmap(reinterpret_cast<HI_VOID*>(yuv), YUVSize);
			}
			HI_MPI_VPSS_ReleaseChnFrame(_channelIndex, 0, &frame);
		}
		else
		{
			HI_MPI_VPSS_ReleaseChnFrame(_channelIndex, 0, &frame);
			break;
		}
	}
#endif // !_WIN32
	return true;
}

bool HisiChannel::DecodeByFFmpeg(const AVPacket* packet, int packetIndex)
{
	if (avcodec_send_packet(_decodeContext, packet) == 0)
	{
		while (true)
		{
			int resultReceive = avcodec_receive_frame(_decodeContext, _yuvFrame);
			if (resultReceive == AVERROR(EAGAIN) || resultReceive == AVERROR_EOF)
			{
				break;
			}
			else if (resultReceive < 0)
			{
				LogPool::Warning(LogEvent::Decode, "receive error", _url);
				return false;
			}
			else if (resultReceive >= 0)
			{
				if (sws_scale(_yuv420spSwsContext, _yuvFrame->data,
					_yuvFrame->linesize, 0, _decodeContext->height,
					_yuv420spFrame->data, _yuv420spFrame->linesize) != 0)
				{
					//_yuvHandler->HandleFrame(_yuv420spBuffer, VideoWidth, VideoHeight, packetIndex);

					if (!_detectChannel->IsBusy(_detectIndex))
					{
						_detectChannel->HandleYUV(_yuv420spBuffer, VideoWidth, VideoHeight, packetIndex, _detectIndex);
					}
				}
			}
		}
	}
	else
	{
		LogPool::Error(LogEvent::Decode, "send error", _url);
		return false;
	}
	return true;
}
