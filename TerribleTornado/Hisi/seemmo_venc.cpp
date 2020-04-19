#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <iostream>
#include <cstring>


#include "sample_comm.h"
#include "seemmo_venc.h"


static pthread_t gs_VencPid1;
static SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara1;
std::mutex vencSendFrame_mtx;
std::mutex vrc_option_mtx;
extern CRtmpClient g_rtmpclient;
//extern AICore *g_aicore;
extern AICore_struct *g_aicore_struct;



int HisiPutH264DataToBuffer2(int index,VENC_STREAM_S *pstStream)
{
	HI_S32 i,j;
	HI_S32 len=0,off=0,len2=2;
	unsigned char *pstr;
	static unsigned char * DataBuf =NULL;
	static int iframe=0;
    int currentchn = 0;
    static int oldchn = currentchn;
	int frame_type = 0;
	for (i = 0; i < pstStream->u32PackCount; i++){
		len+=pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
	}

	if(DataBuf == NULL){
		DataBuf = (unsigned char *)malloc(1024*1024);
	}
	memset(DataBuf,0,1024*1024);
	for (i = 0; i < pstStream->u32PackCount; i++){
		memcpy(DataBuf+off,pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
		off+=pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
		pstr=pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset;
		if(pstr[4]==0x67){
			iframe=1;
		}
	}

	if(iframe == 1){
        //RAW_LOG(INFO,"PTS = %u_____________________\n",pstStream->pstPack[0].u64PTS/90);
        auto &RtmpMgr = g_aicore_struct->getRtmpManager(index);
        RtmpMgr.PutBuffer((unsigned char *)DataBuf,(uint32_t)len,pstStream->pstPack[0].u64PTS/90);
   }
	 return HI_SUCCESS;
}

HI_S32 SEEMMO_COMM_VENC_Creat(VENC_CHN VencChn, PAYLOAD_TYPE_E enType,  PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode, HI_U32  u32Profile, VENC_GOP_ATTR_S *pstGopAttr)
{
    HI_S32 s32Ret;
    SIZE_S stPicSize;
    VENC_CHN_ATTR_S        stVencChnAttr;
    VENC_ATTR_JPEG_S       stJpegAttr;
    SAMPLE_VI_CONFIG_S     stViConfig;
    HI_U32                 u32FrameRate;
    HI_U32                 u32StatTime;
    HI_U32                 u32Gop = 5;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize( enSize, &stPicSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Get picture size failed!\n");
        return HI_FAILURE;
    }

    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);
    if(SAMPLE_SNS_TYPE_BUTT == stViConfig.astViInfo[0].stSnsInfo.enSnsType)
    {
        SAMPLE_PRT("Not set SENSOR%d_TYPE !\n",0);
        return HI_FALSE;
    }
    s32Ret = SAMPLE_COMM_VI_GetFrameRateBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, &u32FrameRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetFrameRateBySensor failed!\n");
        return s32Ret;
    }

    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVencAttr.enType          = enType;
    stVencChnAttr.stVencAttr.u32MaxPicWidth  = stPicSize.u32Width;
    stVencChnAttr.stVencAttr.u32MaxPicHeight = stPicSize.u32Height;
    stVencChnAttr.stVencAttr.u32PicWidth     = stPicSize.u32Width;/*the picture width*/
    stVencChnAttr.stVencAttr.u32PicHeight    = stPicSize.u32Height;/*the picture height*/
    stVencChnAttr.stVencAttr.u32BufSize      = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size*/
    stVencChnAttr.stVencAttr.u32Profile      = u32Profile;
    stVencChnAttr.stVencAttr.bByFrame        = HI_TRUE;/*get stream mode is slice mode or frame mode?*/

    if(VENC_GOPMODE_ADVSMARTP == pstGopAttr->enGopMode)
    {
        u32StatTime = pstGopAttr->stAdvSmartP.u32BgInterval/u32Gop;
    }
    else if(VENC_GOPMODE_SMARTP == pstGopAttr->enGopMode)
    {
        u32StatTime = pstGopAttr->stSmartP.u32BgInterval/u32Gop;
    }
    else
    {
        u32StatTime = 1;
    }

    switch (enType)
    {
        case PT_H265:
        {
            stVencChnAttr.stVencAttr.stAttrH265e.bRcnRefShareBuf = HI_FALSE;
            if (SAMPLE_RC_CBR == enRcMode)
            {
                VENC_H265_CBR_S    stH265Cbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
                stH265Cbr.u32Gop            = u32Gop;
                stH265Cbr.u32StatTime       = u32StatTime; /* stream rate statics time(s) */
                stH265Cbr.u32SrcFrameRate   = u32FrameRate; /* input (vi) frame rate */
                stH265Cbr.fr32DstFrameRate  = u32FrameRate; /* target frame rate */
                switch (enSize)
                {
                    case PIC_720P:
                        stH265Cbr.u32BitRate = 1024 * 2 + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stH265Cbr.u32BitRate = 1024 * 2 + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stH265Cbr.u32BitRate = 1024 * 3 + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stH265Cbr.u32BitRate = 1024 * 5  + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stH265Cbr.u32BitRate = 1024 * 10 + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stH265Cbr.u32BitRate = 1024 * 20 + 5120*u32FrameRate/30;
                        break;
                    default :
                        stH265Cbr.u32BitRate = 1024 * 15 + 2048*u32FrameRate/30;
                        break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stH265Cbr, &stH265Cbr, sizeof(VENC_H265_CBR_S));
            }
            else if (SAMPLE_RC_FIXQP == enRcMode)
            {
                VENC_H265_FIXQP_S    stH265FixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
                stH265FixQp.u32Gop              = 30;
                stH265FixQp.u32SrcFrameRate     = u32FrameRate;
                stH265FixQp.fr32DstFrameRate    = u32FrameRate;
                stH265FixQp.u32IQp              = 25;
                stH265FixQp.u32PQp              = 30;
                stH265FixQp.u32BQp              = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH265FixQp, &stH265FixQp, sizeof(VENC_H265_FIXQP_S));
            }
            else if (SAMPLE_RC_VBR == enRcMode)
            {
                VENC_H265_VBR_S    stH265Vbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
                stH265Vbr.u32Gop           = u32Gop;
                stH265Vbr.u32StatTime      = u32StatTime;
                stH265Vbr.u32SrcFrameRate  = u32FrameRate;
                stH265Vbr.fr32DstFrameRate = u32FrameRate;
                switch (enSize)
                {
                    case PIC_720P:
                        stH265Vbr.u32MaxBitRate = 1024 * 2 + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stH265Vbr.u32MaxBitRate = 1024 * 2 + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stH265Vbr.u32MaxBitRate = 1024 * 3 + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stH265Vbr.u32MaxBitRate = 1024 * 5  + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stH265Vbr.u32MaxBitRate = 1024 * 10 + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stH265Vbr.u32MaxBitRate = 1024 * 20 + 5120*u32FrameRate/30;
                        break;
                    default :
                        stH265Vbr.u32MaxBitRate    = 1024 * 15 + 2048*u32FrameRate/30;
                        break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stH265Vbr, &stH265Vbr, sizeof(VENC_H265_VBR_S));
            }
            else if(SAMPLE_RC_AVBR == enRcMode)
            {
                VENC_H265_AVBR_S    stH265AVbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265AVBR;
                stH265AVbr.u32Gop         = u32Gop;
                stH265AVbr.u32StatTime    = u32StatTime;
                stH265AVbr.u32SrcFrameRate  = u32FrameRate;
                stH265AVbr.fr32DstFrameRate = u32FrameRate;
                switch (enSize)
                {
                    case PIC_720P:
                        stH265AVbr.u32MaxBitRate = 1024 * 2 + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stH265AVbr.u32MaxBitRate = 1024 * 2 + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stH265AVbr.u32MaxBitRate = 1024 * 3 + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stH265AVbr.u32MaxBitRate = 1024 * 5  + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stH265AVbr.u32MaxBitRate = 1024 * 10 + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stH265AVbr.u32MaxBitRate = 1024 * 20 + 5120*u32FrameRate/30;
                        break;
                    default :
                        stH265AVbr.u32MaxBitRate    = 1024 * 15 + 2048*u32FrameRate/30;
                        break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stH265AVbr, &stH265AVbr, sizeof(VENC_H265_AVBR_S));
            }
            else if(SAMPLE_RC_QPMAP == enRcMode)
            {
                VENC_H265_QPMAP_S    stH265QpMap;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265QPMAP;
                stH265QpMap.u32Gop           = u32Gop;
                stH265QpMap.u32StatTime      = u32StatTime;
                stH265QpMap.u32SrcFrameRate  = u32FrameRate;
                stH265QpMap.fr32DstFrameRate = u32FrameRate;
                stH265QpMap.enQpMapMode      = VENC_RC_QPMAP_MODE_MEANQP;
                memcpy(&stVencChnAttr.stRcAttr.stH265QpMap, &stH265QpMap, sizeof(VENC_H265_QPMAP_S));
            }
            else
            {
                SAMPLE_PRT("%s,%d,enRcMode(%d) not support\n",__FUNCTION__,__LINE__,enRcMode);
                return HI_FAILURE;
            }
        }
        break;
        case PT_H264:
        {
            stVencChnAttr.stVencAttr.stAttrH264e.bRcnRefShareBuf = HI_FALSE;
            if (SAMPLE_RC_CBR == enRcMode)
            {
                VENC_H264_CBR_S    stH264Cbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stH264Cbr.u32Gop                = u32Gop; /*the interval of IFrame*/
                stH264Cbr.u32StatTime           = u32StatTime; /* stream rate statics time(s) */
                stH264Cbr.u32SrcFrameRate       = u32FrameRate; /* input (vi) frame rate */
                stH264Cbr.fr32DstFrameRate      = u32FrameRate; /* target frame rate */
                switch (enSize)
                {
                    case PIC_720P:
                        stH264Cbr.u32BitRate = 1024 * 2  + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stH264Cbr.u32BitRate = 1024 * 2  + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stH264Cbr.u32BitRate = 1024 * 3  + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stH264Cbr.u32BitRate = 1024 * 5 + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stH264Cbr.u32BitRate = 1024 * 12 + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stH264Cbr.u32BitRate = 1024 * 10 + 5120*u32FrameRate/30;
                        break;
                    default :
                        stH264Cbr.u32BitRate = 1024 * 15 + 2048*u32FrameRate/30;
                        break;
                }

                memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(VENC_H264_CBR_S));
            }
            else if (SAMPLE_RC_FIXQP == enRcMode)
            {
                VENC_H264_FIXQP_S    stH264FixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop           = 30;
                stH264FixQp.u32SrcFrameRate  = u32FrameRate;
                stH264FixQp.fr32DstFrameRate = u32FrameRate;
                stH264FixQp.u32IQp           = 25;
                stH264FixQp.u32PQp           = 30;
                stH264FixQp.u32BQp           = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH264FixQp, &stH264FixQp, sizeof(VENC_H264_FIXQP_S));
            }
            else if (SAMPLE_RC_VBR == enRcMode)
            {
                VENC_H264_VBR_S    stH264Vbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stH264Vbr.u32Gop           = u32Gop;
                stH264Vbr.u32StatTime      = u32StatTime;
                stH264Vbr.u32SrcFrameRate  = u32FrameRate;
                stH264Vbr.fr32DstFrameRate = u32FrameRate;
                switch (enSize)
                {
                    case PIC_720P:
                        stH264Vbr.u32MaxBitRate = 1024 * 2   + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stH264Vbr.u32MaxBitRate = 1024 * 2   + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stH264Vbr.u32MaxBitRate = 1024 * 3   + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stH264Vbr.u32MaxBitRate = 1024 * 5   + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stH264Vbr.u32MaxBitRate = 1024 * 10  + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stH264Vbr.u32MaxBitRate = 1024 * 20  + 5120*u32FrameRate/30;
                        break;
                    default :
                        stH264Vbr.u32MaxBitRate = 1024 * 15  + 2048*u32FrameRate/30;
                        break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stH264Vbr, &stH264Vbr, sizeof(VENC_H264_VBR_S));
            }
            else if (SAMPLE_RC_AVBR == enRcMode)
            {
                VENC_H264_VBR_S    stH264AVbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264AVBR;
                stH264AVbr.u32Gop           = u32Gop;
                stH264AVbr.u32StatTime      = u32StatTime;
                stH264AVbr.u32SrcFrameRate  = u32FrameRate;
                stH264AVbr.fr32DstFrameRate = u32FrameRate;
                switch (enSize)
                {
                    case PIC_720P:
                        stH264AVbr.u32MaxBitRate = 1024 * 2   + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stH264AVbr.u32MaxBitRate = 1024 * 2   + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stH264AVbr.u32MaxBitRate = 1024 * 3   + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stH264AVbr.u32MaxBitRate = 1024 * 5   + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stH264AVbr.u32MaxBitRate = 1024 * 10  + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stH264AVbr.u32MaxBitRate = 1024 * 20  + 5120*u32FrameRate/30;
                        break;
                    default :
                        stH264AVbr.u32MaxBitRate = 1024 * 15  + 2048*u32FrameRate/30;
                        break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stH264AVbr, &stH264AVbr, sizeof(VENC_H264_AVBR_S));
            }
            else if (SAMPLE_RC_AVBR == enRcMode)
            {
                VENC_H264_VBR_S    stH264AVbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264AVBR;
                stH264AVbr.u32Gop           = u32Gop;
                stH264AVbr.u32StatTime      = u32StatTime;
                stH264AVbr.u32SrcFrameRate  = u32FrameRate;
                stH264AVbr.fr32DstFrameRate = u32FrameRate;
                switch (enSize)
                {
                    case PIC_720P:
                        stH264AVbr.u32MaxBitRate = 1024 * 2   + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stH264AVbr.u32MaxBitRate = 1024 * 2   + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stH264AVbr.u32MaxBitRate = 1024 * 3   + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stH264AVbr.u32MaxBitRate = 1024 * 5   + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stH264AVbr.u32MaxBitRate = 1024 * 10  + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stH264AVbr.u32MaxBitRate = 1024 * 20  + 5120*u32FrameRate/30;
                        break;
                    default :
                        stH264AVbr.u32MaxBitRate = 1024 * 15  + 2048*u32FrameRate/30;
                        break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stH264AVbr, &stH264AVbr, sizeof(VENC_H264_AVBR_S));
            }
            else if(SAMPLE_RC_QPMAP == enRcMode)
            {
                VENC_H264_QPMAP_S    stH264QpMap;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264QPMAP;
                stH264QpMap.u32Gop           = u32Gop;
                stH264QpMap.u32StatTime      = u32StatTime;
                stH264QpMap.u32SrcFrameRate  = u32FrameRate;
                stH264QpMap.fr32DstFrameRate = u32FrameRate;
                memcpy(&stVencChnAttr.stRcAttr.stH264QpMap, &stH264QpMap, sizeof(VENC_H264_QPMAP_S));
            }
            else
            {
                SAMPLE_PRT("%s,%d,enRcMode(%d) not support\n",__FUNCTION__,__LINE__,enRcMode);
                return HI_FAILURE;
            }
        }
        break;
        case PT_MJPEG:
        {
            if (SAMPLE_RC_FIXQP == enRcMode)
            {
                VENC_MJPEG_FIXQP_S stMjpegeFixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
                stMjpegeFixQp.u32Qfactor        = 95;
                stMjpegeFixQp.u32SrcFrameRate    = u32FrameRate;
                stMjpegeFixQp.fr32DstFrameRate   = u32FrameRate;

                memcpy(&stVencChnAttr.stRcAttr.stMjpegFixQp, &stMjpegeFixQp,sizeof(VENC_MJPEG_FIXQP_S));
            }
            else if (SAMPLE_RC_CBR == enRcMode)
            {
                VENC_MJPEG_CBR_S stMjpegeCbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
                stMjpegeCbr.u32StatTime         = u32StatTime;
                stMjpegeCbr.u32SrcFrameRate     = u32FrameRate;
                stMjpegeCbr.fr32DstFrameRate    = u32FrameRate;
                switch (enSize)
                {
                    case PIC_720P:
                        stMjpegeCbr.u32BitRate = 1024 * 5  + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stMjpegeCbr.u32BitRate = 1024 * 8  + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stMjpegeCbr.u32BitRate = 1024 * 20 + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stMjpegeCbr.u32BitRate = 1024 * 25 + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stMjpegeCbr.u32BitRate = 1024 * 30 + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stMjpegeCbr.u32BitRate = 1024 * 40 + 5120*u32FrameRate/30;
                        break;
                    default :
                        stMjpegeCbr.u32BitRate = 1024 * 20 + 2048*u32FrameRate/30;
                        break;
                }

                memcpy(&stVencChnAttr.stRcAttr.stMjpegCbr, &stMjpegeCbr,sizeof(VENC_MJPEG_CBR_S));
            }
            else if ((SAMPLE_RC_VBR == enRcMode)||(SAMPLE_RC_AVBR == enRcMode))
            {
                VENC_MJPEG_VBR_S   stMjpegVbr;

                if(SAMPLE_RC_AVBR == enRcMode)
                {
                    SAMPLE_PRT("Mjpege not support AVBR, so change rcmode to VBR!\n");
                }

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
                stMjpegVbr.u32StatTime      = u32StatTime;
                stMjpegVbr.u32SrcFrameRate  = u32FrameRate;
                stMjpegVbr.fr32DstFrameRate = 5;

                switch (enSize)
                {
                    case PIC_720P:
                        stMjpegVbr.u32MaxBitRate = 1024 * 5 + 1024*u32FrameRate/30;
                        break;
                    case PIC_1080P:
                        stMjpegVbr.u32MaxBitRate = 1024 * 8 + 2048*u32FrameRate/30;
                        break;
                    case PIC_2592x1944:
                        stMjpegVbr.u32MaxBitRate = 1024 * 20 + 3072*u32FrameRate/30;
                        break;
                    case PIC_3840x2160:
                        stMjpegVbr.u32MaxBitRate = 1024 * 25 + 5120*u32FrameRate/30;
                        break;
                    case PIC_4000x3000:
                        stMjpegVbr.u32MaxBitRate    = 1024 * 30 + 5120*u32FrameRate/30;
                        break;
                    case PIC_7680x4320:
                        stMjpegVbr.u32MaxBitRate = 1024 * 40 + 5120*u32FrameRate/30;
                        break;
                    default :
                        stMjpegVbr.u32MaxBitRate = 1024 * 20 + 2048*u32FrameRate/30;
                        break;
                }

                memcpy(&stVencChnAttr.stRcAttr.stMjpegVbr, &stMjpegVbr,sizeof(VENC_MJPEG_VBR_S));
            }
            else
            {
                SAMPLE_PRT("cann't support other mode(%d) in this version!\n",enRcMode);
                return HI_FAILURE;
            }
        }
        break;

        case PT_JPEG:
            stJpegAttr.bSupportDCF     = HI_FALSE;
            stJpegAttr.stMPFCfg.u8LargeThumbNailNum = 0;
            memcpy(&stVencChnAttr.stVencAttr.stAttrJpege, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));
        break;

        case PT_PRORES:
            stVencChnAttr.stVencAttr.stAttrProres.cIdentifier[0] = 'h';
            stVencChnAttr.stVencAttr.stAttrProres.cIdentifier[1] = 'i';
            stVencChnAttr.stVencAttr.stAttrProres.cIdentifier[2] = 's';
            stVencChnAttr.stVencAttr.stAttrProres.cIdentifier[3] = 'i';
            if(120 == u32FrameRate)
            {
                stVencChnAttr.stVencAttr.stAttrProres.enFrameRateCode = PRORES_FR_120;
            }
            else if(60 == u32FrameRate)
            {
                stVencChnAttr.stVencAttr.stAttrProres.enFrameRateCode = PRORES_FR_60;
            }
            else
            {
                stVencChnAttr.stVencAttr.stAttrProres.enFrameRateCode = PRORES_FR_30;
            }
            stVencChnAttr.stVencAttr.stAttrProres.enAspectRatio   = PRORES_ASPECT_RATIO_16_9;
        break;

        default:
            SAMPLE_PRT("cann't support this enType (%d) in this version!\n",enType);
            return HI_ERR_VENC_NOT_SUPPORT;
    }

    if(PT_MJPEG == enType || PT_JPEG == enType || PT_PRORES == enType)
    {
        stVencChnAttr.stGopAttr.enGopMode  = VENC_GOPMODE_NORMALP;
        stVencChnAttr.stGopAttr.stNormalP.s32IPQpDelta = 0;
    }
    else
    {
        memcpy(&stVencChnAttr.stGopAttr,pstGopAttr,sizeof(VENC_GOP_ATTR_S));
        if((VENC_GOPMODE_ADVSMARTP == pstGopAttr->enGopMode)&&(PT_H264 == enType))
        {
            stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_SMARTP;

            SAMPLE_PRT("H.264 not support advsmartp, so change gopmode to smartp!\n");
        }

        if((VENC_GOPMODE_BIPREDB == pstGopAttr->enGopMode)&&(PT_H264 == enType))
        {
            if(0 == stVencChnAttr.stVencAttr.u32Profile)
            {
                stVencChnAttr.stVencAttr.u32Profile = 1;

                SAMPLE_PRT("H.264 base profile not support BIPREDB, so change profile to main profile!\n");
            }
        }

        if((VENC_RC_MODE_H264QPMAP == stVencChnAttr.stRcAttr.enRcMode)||(VENC_RC_MODE_H265QPMAP == stVencChnAttr.stRcAttr.enRcMode))
        {
            if(VENC_GOPMODE_ADVSMARTP == pstGopAttr->enGopMode)
            {
                stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_SMARTP;

                SAMPLE_PRT("advsmartp not support QPMAP, so change gopmode to smartp!\n");
            }
        }
    }

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x! ===\n", \
                   VencChn, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : Start venc stream mode
* note      : rate control parameter need adjust, according your case.
******************************************************************************/
HI_S32 SEEMMO_COMM_VENC_Start(VENC_CHN VencChn, PAYLOAD_TYPE_E enType,  PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode, HI_U32  u32Profile, VENC_GOP_ATTR_S *pstGopAttr)
{
    HI_S32 s32Ret;
    VENC_RECV_PIC_PARAM_S  stRecvParam;

    /******************************************
     step 1:  Creat Encode Chnl
    ******************************************/
    s32Ret = SEEMMO_COMM_VENC_Creat(VencChn,enType,enSize,enRcMode,u32Profile,pstGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_Creat faild with%#x! \n", s32Ret);
        return HI_FAILURE;
    }
    /******************************************
     step 2:  Start Recv Venc Pictures
    ******************************************/
    stRecvParam.s32RecvPicNum = -1;
    s32Ret = HI_MPI_VENC_StartRecvFrame(VencChn,&stRecvParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x! \n", s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}


/******************************************************************************
* funciton : get stream from each channels and save them
******************************************************************************/
HI_VOID* SEEMMO_COMM_VENC_GetVencStreamProc(HI_VOID* p)
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
    VENC_CHN_ATTR_S stVencChnAttr;
    SAMPLE_VENC_GETSTREAM_PARA_S* pstPara;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_U32 u32PictureCnt[VENC_MAX_CHN_NUM]={0};
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    HI_CHAR aszFileName[VENC_MAX_CHN_NUM][64];
    FILE* pFile[VENC_MAX_CHN_NUM];
    char szFilePostfix[10];
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    VENC_CHN VencChn;
    PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
    VENC_STREAM_BUF_INFO_S stStreamBufInfo[VENC_MAX_CHN_NUM];

    prctl(PR_SET_NAME, "GetVencStream", 0,0,0);

    pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S*)p;
    s32ChnTotal = pstPara->s32Cnt;
    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    if (s32ChnTotal >= VENC_MAX_CHN_NUM)
    {
        SAMPLE_PRT("input count invaild\n");
        return NULL;
    }
    for (i = 0; i < s32ChnTotal; i++)
    {
        /* decide the stream file name, and open file to save stream */
        VencChn = pstPara->VeChn[i];
        s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
                       VencChn, s32Ret);
            return NULL;
        }
        enPayLoadType[i] = stVencChnAttr.stVencAttr.enType;

        /* Set Venc Fd. */
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] < 0)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                       VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }

        s32Ret = HI_MPI_VENC_GetStreamBufInfo (i, &stStreamBufInfo[i]);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetStreamBufInfo failed with %#x!\n", s32Ret);
            return (void *)HI_FAILURE;
        }
    }

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (HI_TRUE == pstPara->bThreadStart)
    {
        FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 0;
        TimeoutVal.tv_usec = 10000;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            SAMPLE_PRT("select failed!\n");
            break;
        }
        else if (s32Ret == 0)
        {
            //SAMPLE_PRT("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
            for (i = 0; i < s32ChnTotal; i++)
            {
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    /*******************************************************
                     step 2.1 : query how many packs in one-frame stream.
                    *******************************************************/
                    memset(&stStream, 0, sizeof(stStream));

                    s32Ret = HI_MPI_VENC_QueryStatus(i, &stStat);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_QueryStatus chn[%d] failed with %#x!\n", i, s32Ret);
                        break;
                    }

                    /*******************************************************
                    step 2.2 :suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
                     if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
                     {
                        SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
                        continue;
                     }
                    *******************************************************/
                    if(0 == stStat.u32CurPacks)
                    {
                          SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
                          continue;
                    }
                    /*******************************************************
                     step 2.3 : malloc corresponding number of pack nodes.
                    *******************************************************/
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        SAMPLE_PRT("malloc stream pack failed!\n");
                        break;
                    }

                    /*******************************************************
                     step 2.4 : call mpi to get one-frame stream
                    *******************************************************/
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                                   s32Ret);
                        break;
                    }

                    

                    /*******************************************************
                     step 2.5 : save frame to file
                    *******************************************************/

                    // printf("+++++++++++++++++++ get channel %d 's stream +++++++++++++++++++\n", i);
                    HisiPutH264DataToBuffer2(i,&stStream);

                    /*******************************************************
                     step 2.6 : release stream
                     *******************************************************/
                    s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_ReleaseStream failed!\n");
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }

                    /*******************************************************
                     step 2.7 : free pack nodes
                    *******************************************************/
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    u32PictureCnt[i]++;
                    if(PT_JPEG == enPayLoadType[i])
                    {
                        fclose(pFile[i]);
                    }
                }
            }
        }
    }
    /*******************************************************
    * step 3 : close save-file
    *******************************************************/
    #if 0
    for (i = 0; i < s32ChnTotal; i++)
    {
        if(PT_JPEG != enPayLoadType[i])
        {
            fclose(pFile[i]);
        }
    }
    #endif
    return NULL;
}

/******************************************************************************
* funciton : start get venc stream process thread
******************************************************************************/
HI_S32 SEEMMO_COMM_VENC_StartGetStream(VENC_CHN VeChn[],HI_S32 s32Cnt)
{
    HI_U32 i;

    gs_stPara1.bThreadStart = HI_TRUE;
    gs_stPara1.s32Cnt = s32Cnt;
    for(i=0; i<s32Cnt; i++)
    {
        gs_stPara1.VeChn[i] = VeChn[i];
    }
    return pthread_create(&gs_VencPid1, 0, SEEMMO_COMM_VENC_GetVencStreamProc, (HI_VOID*)&gs_stPara1);
}


/******************************************************************************
* funciton : stop get venc stream process.
******************************************************************************/
HI_S32 SEEMMO_COMM_VENC_StopGetStream(void)
{
    if (HI_TRUE == gs_stPara1.bThreadStart)
    {
        gs_stPara1.bThreadStart = HI_FALSE;
        pthread_join(gs_VencPid1, 0);
    }
    return HI_SUCCESS;
}

