

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "vo_layout.h"
#include "loadbmp.h"

//extern int kbHaveLogo;
//extern VIDEO_FRAME_INFO_S g_logo_frame;
//extern SAMPLE_VO_MODE_E g_vo_mode;


static HI_BOOL s_bHaveRegion = HI_FALSE;

HI_S32 SEEMMO_LoadRgnBmp(const char *filename, BITMAP_S *pstBitmap, HI_BOOL bFil, HI_U32 u16FilColor)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if(GetBmpInfo(filename,&bmpFileHeader,&bmpInfo) < 0)
    {
		printf("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;

    pstBitmap->pData = malloc(2*(bmpInfo.bmiHeader.biWidth)*(bmpInfo.bmiHeader.biHeight));

    if(NULL == pstBitmap->pData)
    {
        printf("malloc osd memroy err!\n");
        return HI_FAILURE;
    }
    CreateSurfaceByBitMap(filename,&Surface,(HI_U8*)(pstBitmap->pData));

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = PIXEL_FORMAT_ARGB_1555;

    int i,j;
    HI_U16 *pu16Temp;
    pu16Temp = (HI_U16*)pstBitmap->pData;

    if (bFil)
    {
        for (i=0; i<pstBitmap->u32Height; i++)
        {
            for (j=0; j<pstBitmap->u32Width; j++)
            {
                if (u16FilColor == *pu16Temp)
                {
                    *pu16Temp &= 0x7FFF;
                }
                pu16Temp++;
            }
        }

    }

    return HI_SUCCESS;
}

HI_S32 SEEMMO_CreateRegion(HI_S32 s32RegionId, char* bmpfile)
{
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    BITMAP_S stBitmap;
    VIDEO_FRAME_INFO_S stNulFrame;

    memset(&stNulFrame, 0, sizeof(stNulFrame));

    /* creat region*/
    stRgnAttr.enType = OVERLAYEX_RGN;
    stRgnAttr.unAttr.stOverlayEx.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Width  = 640;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Height = 360;
    stRgnAttr.unAttr.stOverlayEx.u32BgColor = 0xfc;
    stRgnAttr.unAttr.stOverlayEx.u32CanvasNum = 2;

    s32Ret = HI_MPI_RGN_Create(s32RegionId, &stRgnAttr);
    if (s32Ret != HI_SUCCESS)
    {
	printf("region %d create fail. value=0x%x.", s32RegionId, s32Ret);
	return s32Ret;
    }

    /* load bitmap*/
    s32Ret = SEEMMO_LoadRgnBmp(bmpfile, &stBitmap, HI_FALSE, 0);
    if (s32Ret != HI_SUCCESS)
    {
	return s32Ret;
    }

    s32Ret = HI_MPI_RGN_SetBitMap(s32RegionId, &stBitmap);
    if (s32Ret != HI_SUCCESS)
    {
	printf("region set bitmap to region %d fail. value=0x%x.", s32RegionId, s32Ret);
	free(stBitmap.pData);
	return s32Ret;
    }
    free(stBitmap.pData);

    /*attach region to chn*/
    stChn.enModId = HI_ID_VO;
    stChn.s32DevId = SAMPLE_VO_DEV_UHD;
    stChn.s32ChnId = 0;

    stChnAttr.bShow = HI_TRUE;
    stChnAttr.enType = OVERLAYEX_RGN;
    stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 640;
    stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 360;
    stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 128;
    stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 128;
    stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = 0;

    s32Ret = HI_MPI_RGN_AttachToChn(s32RegionId, &stChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
	printf("region %d attach to VO fail. value=0x%x.", s32RegionId, s32Ret);
	return s32Ret;
    }

    s32Ret = HI_MPI_VO_SendFrame(0, 8, &stNulFrame, 50);
    if (s32Ret != HI_SUCCESS)
    {
	printf("send null frame to vo failed, ret = 0x%x.", s32Ret);
	return s32Ret;
    }
    return HI_SUCCESS;
}

HI_S32 SEEMMO_COMM_VO_StartChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode, SEEMMO_LAYOUT_MODE_E enLayoutMode)
{
    HI_S32 i;
    HI_S32 s32Ret    = HI_SUCCESS;
    HI_S32 ret       = HI_SUCCESS;
    HI_U32 u32WndNum = 0;
    HI_U32 u32Square = 0;
    HI_U32 u32Row    = 0;
    HI_U32 u32Col    = 0;
    HI_U32 u32Width  = 0;
    HI_U32 u32Height = 0;
    VO_CHN_ATTR_S         stChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;

    switch (enMode)
    {
        case VO_MODE_1MUX:
            u32WndNum = 1;
            u32Square = 1;
            break;
        case VO_MODE_2MUX:
            u32WndNum = 2;
            u32Square = 2;
            break;
        case VO_MODE_4MUX:
            u32WndNum = 4;
            u32Square = 2;
            break;
        case VO_MODE_8MUX:
            u32WndNum = 8;
            u32Square = 3;
            break;
        case VO_MODE_9MUX:
            u32WndNum = 9;
            u32Square = 3;
            break;
        case VO_MODE_16MUX:
            u32WndNum = 16;
            u32Square = 4;
            break;
        case VO_MODE_25MUX:
            u32WndNum = 25;
            u32Square = 5;
            break;
        case VO_MODE_36MUX:
            u32WndNum = 36;
            u32Square = 6;
            break;
        case VO_MODE_49MUX:
            u32WndNum = 49;
            u32Square = 7;
            break;
        case VO_MODE_64MUX:
            u32WndNum = 64;
            u32Square = 8;
            break;
        case VO_MODE_2X4:
            u32WndNum = 8;
            u32Square = 3;
            u32Row    = 4;
            u32Col    = 2;
            break;
        default:
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    u32Width  = stLayerAttr.stImageSize.u32Width;
    u32Height = stLayerAttr.stImageSize.u32Height;
    SAMPLE_PRT("u32Width:%d, u32Height:%d, u32Square:%d\n", u32Width, u32Height, u32Square);

#if 1 // add by lightspot for 8mux 1+7 layout
    if(enMode == VO_MODE_8MUX && enLayoutMode == SEEMMO_LAYOUT_7_1) {
	stChnAttr.stRect.s32X       = 0;
	stChnAttr.stRect.s32Y       = 0;
	stChnAttr.stRect.u32Width   = 480*3;
	stChnAttr.stRect.u32Height  = 270*3;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 0, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 0);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 480*3;
	stChnAttr.stRect.s32Y       = 0;
	stChnAttr.stRect.u32Width   = 480;
	stChnAttr.stRect.u32Height  = 270;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 1, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 1);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 480*3;
	stChnAttr.stRect.s32Y       = 270*1;
	stChnAttr.stRect.u32Width   = 480; 
	stChnAttr.stRect.u32Height  = 270; 
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 2, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}    
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 2);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}    

	stChnAttr.stRect.s32X       = 480*3;
	stChnAttr.stRect.s32Y       = 270*2;
	stChnAttr.stRect.u32Width   = 480; 
	stChnAttr.stRect.u32Height  = 270; 
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 3, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}    
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 3);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 0;
	stChnAttr.stRect.s32Y       = 270*3;
	stChnAttr.stRect.u32Width   = 480;
	stChnAttr.stRect.u32Height  = 270;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 4, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 4);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 480*1;
	stChnAttr.stRect.s32Y       = 270*3;
	stChnAttr.stRect.u32Width   = 480;
	stChnAttr.stRect.u32Height  = 270;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 5, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 5);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 480*2;
	stChnAttr.stRect.s32Y       = 270*3;
	stChnAttr.stRect.u32Width   = 480;
	stChnAttr.stRect.u32Height  = 270;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 6, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 6);
	if (s32Ret != HI_SUCCESS) {
	            SAMPLE_PRT("failed with %#x!\n", s32Ret);
		            return HI_FAILURE;
			        }

	stChnAttr.stRect.s32X       = 480*3;
	stChnAttr.stRect.s32Y       = 270*3;
	stChnAttr.stRect.u32Width   = 480;
	stChnAttr.stRect.u32Height  = 270;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 7, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 7);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

#if 0
    if(kbHaveLogo) {
        for(int i=0;i<u32WndNum;i++)
        {
            ret = HI_MPI_VO_SendFrame(0, i, &g_logo_frame, 1000);
            if(0 != ret) {
                printf("Send logo_frame to vo [%d] failed. ret = [%#x]\n", i,ret);
            }
        }
    }
#endif
	return HI_SUCCESS;
    }
#endif
#if 1 // add by lightspot for 8mux loop layout
    if(enMode == VO_MODE_9MUX && enLayoutMode == SEEMMO_LAYOUT_LOOP) {
	stChnAttr.stRect.s32X       = 0;
	stChnAttr.stRect.s32Y       = 0;
	stChnAttr.stRect.u32Width   = 640; 
	stChnAttr.stRect.u32Height  = 360; 
	stChnAttr.u32Priority       = 0; 
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 0, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 0);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 640;
	stChnAttr.stRect.s32Y       = 0;
	stChnAttr.stRect.u32Width   = 640; 
	stChnAttr.stRect.u32Height  = 360; 
	stChnAttr.u32Priority       = 0; 
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 1, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 1);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 640*2;
	stChnAttr.stRect.s32Y       = 0;
	stChnAttr.stRect.u32Width   = 640;
	stChnAttr.stRect.u32Height  = 360;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 2, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 2);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 0;
	stChnAttr.stRect.s32Y       = 360;
	stChnAttr.stRect.u32Width   = 640;
	stChnAttr.stRect.u32Height  = 360;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 3, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 3);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 640*2;
	stChnAttr.stRect.s32Y       = 360;
	stChnAttr.stRect.u32Width   = 640;
	stChnAttr.stRect.u32Height  = 360;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 4, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 4);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 0;
	stChnAttr.stRect.s32Y       = 360*2;
	stChnAttr.stRect.u32Width   = 640;
	stChnAttr.stRect.u32Height  = 360;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 5, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 5);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 640;
	stChnAttr.stRect.s32Y       = 360*2;
	stChnAttr.stRect.u32Width   = 640;
	stChnAttr.stRect.u32Height  = 360;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 6, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 6);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 640*2;
	stChnAttr.stRect.s32Y       = 360*2;
	stChnAttr.stRect.u32Width   = 640;
	stChnAttr.stRect.u32Height  = 360;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 7, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 7);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

	stChnAttr.stRect.s32X       = 640;
	stChnAttr.stRect.s32Y       = 360;
	stChnAttr.stRect.u32Width   = 640;
	stChnAttr.stRect.u32Height  = 360;
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, 8, &stChnAttr);
	if (s32Ret != HI_SUCCESS) {
	    printf("%s(%d):failed with %#x!\n", \
		    __FUNCTION__, __LINE__,  s32Ret);
	    return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, 8);
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}
#if 0
	s32Ret = SEEMMO_CreateRegion(0, "./logo.bmp");
	if (s32Ret != HI_SUCCESS) {
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return s32Ret;
	}
#endif

#if 0
    if(kbHaveLogo) {
        for(int i=0;i<u32WndNum;i++)
        {
            ret = HI_MPI_VO_SendFrame(0, i, &g_logo_frame, 1000);
            if(0 != ret) {
                printf("Send logo_frame to vo [%d] failed. ret = [%#x]\n", i,ret);
            }
        }
    }
#endif

	return HI_SUCCESS;
    }
#endif

    for (i = 0; i < u32WndNum; i++)
    {
        if( enMode == VO_MODE_1MUX  ||
            enMode == VO_MODE_2MUX  ||
            enMode == VO_MODE_4MUX  ||
            enMode == VO_MODE_8MUX  ||
            enMode == VO_MODE_9MUX  ||
            enMode == VO_MODE_16MUX ||
            enMode == VO_MODE_25MUX ||
            enMode == VO_MODE_36MUX ||
            enMode == VO_MODE_49MUX ||
            enMode == VO_MODE_64MUX )
        {
            stChnAttr.stRect.s32X       = ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 2);
            stChnAttr.stRect.s32Y       = ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 2);
            stChnAttr.stRect.u32Width   = ALIGN_DOWN(u32Width / u32Square, 2);
            stChnAttr.stRect.u32Height  = ALIGN_DOWN(u32Height / u32Square, 2);
            stChnAttr.u32Priority       = 0;
            stChnAttr.bDeflicker        = HI_FALSE;
        }
        else if(enMode == VO_MODE_2X4)
        {
            stChnAttr.stRect.s32X       = ALIGN_DOWN((u32Width / u32Col) * (i % u32Col), 2);
            stChnAttr.stRect.s32Y       = ALIGN_DOWN((u32Height / u32Row) * (i / u32Col), 2);
            stChnAttr.stRect.u32Width   = ALIGN_DOWN(u32Width / u32Col, 2);
            stChnAttr.stRect.u32Height  = ALIGN_DOWN(u32Height / u32Row, 2);
            stChnAttr.u32Priority       = 0;
            stChnAttr.bDeflicker        = HI_FALSE;
        }

        s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            printf("%s(%d):failed with %#x!\n", \
                   __FUNCTION__, __LINE__,  s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

#if 0
    if(kbHaveLogo) {
        for(int i=0;i<u32WndNum;i++)
        {
            ret = HI_MPI_VO_SendFrame(0, i, &g_logo_frame, 1000);
            if(0 != ret) {
                printf("Send logo_frame to vo [%d] failed. ret = [%#x]\n", i,ret);
            }
        }
    }
#endif
    return HI_SUCCESS;
}

HI_S32 SEEMMO_COMM_VO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig, SEEMMO_LAYOUT_MODE_E enLayoutMode)
{
    RECT_S                 stDefDispRect  = {0, 0, 1920, 1080};
    SIZE_S                 stDefImageSize = {1920, 1080};

    /*******************************************
    * VO device VoDev# information declaration.
    ********************************************/
    VO_DEV                 VoDev          = 0;
    VO_LAYER               VoLayer        = 0;
    SAMPLE_VO_MODE_E       enVoMode       = 0;
    VO_INTF_TYPE_E         enVoIntfType   = VO_INTF_HDMI;
    VO_INTF_SYNC_E         enIntfSync     = VO_OUTPUT_1080P30;
    DYNAMIC_RANGE_E        enDstDyRg      = DYNAMIC_RANGE_SDR8;
    VO_PART_MODE_E         enVoPartMode   = VO_PART_MODE_SINGLE;
    VO_PUB_ATTR_S          stVoPubAttr    = {0};
    VO_VIDEO_LAYER_ATTR_S  stLayerAttr    = {0};
    VO_CSC_S               stVideoCSC     = {0};
    HI_S32                 s32Ret         = HI_SUCCESS;

    if (NULL == pstVoConfig)
    {
        SAMPLE_PRT("Error:argument can not be NULL\n");
        return HI_FAILURE;
    }
    VoDev          = pstVoConfig->VoDev;
    VoLayer        = pstVoConfig->VoDev;
    enVoMode       = pstVoConfig->enVoMode;
    enVoIntfType   = pstVoConfig->enVoIntfType;
    enIntfSync     = pstVoConfig->enIntfSync;
    enDstDyRg      = pstVoConfig->enDstDynamicRange;
    enVoPartMode   = pstVoConfig->enVoPartMode;

    /********************************
    * Set and start VO device VoDev#.
    *********************************/
    stVoPubAttr.enIntfType  = enVoIntfType;
    stVoPubAttr.enIntfSync  = enIntfSync;

    stVoPubAttr.u32BgColor  = pstVoConfig->u32BgColor;

    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
        return s32Ret;
    }

    /******************************
    * Set and start layer VoDev#.
    ********************************/

    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync,
                                  &stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height,
                                  &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_GetWH failed!\n");
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }
    stLayerAttr.bClusterMode     = HI_FALSE;
    stLayerAttr.bDoubleFrame    = HI_FALSE;
    stLayerAttr.enPixFormat       = pstVoConfig->enPixFormat;

    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;

    /******************************
    // Set display rectangle if changed.
    ********************************/
    if (0 != memcmp(&pstVoConfig->stDispRect, &stDefDispRect, sizeof(RECT_S)))
    {
        stLayerAttr.stDispRect = pstVoConfig->stDispRect;
    }
    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    /******************************
    //Set image size if changed.
    ********************************/
    if (0 != memcmp(&pstVoConfig->stImageSize, &stDefImageSize, sizeof(SIZE_S)))
    {
        stLayerAttr.stImageSize = pstVoConfig->stImageSize;
    }
    stLayerAttr.enDstDynamicRange     = pstVoConfig->enDstDynamicRange;


    if (VO_PART_MODE_MULTI == enVoPartMode)
    {
        s32Ret = HI_MPI_VO_SetVideoLayerPartitionMode(VoLayer, enVoPartMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VO_SetVideoLayerPartitionMode failed!\n");
            SAMPLE_COMM_VO_StopDev(VoDev);
            return s32Ret;
        }
    }

    if (pstVoConfig->u32DisBufLen)
    {
        s32Ret = HI_MPI_VO_SetDisplayBufLen(VoLayer, pstVoConfig->u32DisBufLen);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VO_SetDisplayBufLen failed with %#x!\n",s32Ret);
            SAMPLE_COMM_VO_StopDev(VoDev);
            return s32Ret;
        }
    }

    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_Start video layer failed!\n");
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }

    if(VO_INTF_MIPI == enVoIntfType)
    {
        s32Ret = HI_MPI_VO_GetVideoLayerCSC(VoLayer, &stVideoCSC);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VO_GetVideoLayerCSC failed!\n");
            SAMPLE_COMM_VO_StopDev(VoDev);
            return s32Ret;
        }
        stVideoCSC.enCscMatrix =VO_CSC_MATRIX_BT709_TO_RGB_PC;
        s32Ret = HI_MPI_VO_SetVideoLayerCSC(VoLayer, &stVideoCSC);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VO_SetVideoLayerCSC failed!\n");
            SAMPLE_COMM_VO_StopDev(VoDev);
            return s32Ret;
        }
    }

    /******************************
    * start vo channels.
    ********************************/
    //s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
    s32Ret = SEEMMO_COMM_VO_StartChn(VoLayer, enVoMode, enLayoutMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
        SAMPLE_COMM_VO_StopLayer(VoLayer);
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }

    /******************************
    * Start hdmi device.
    * Note : do this after vo device started.
    ********************************/
    if(VO_INTF_HDMI == enVoIntfType)
    {
        SAMPLE_COMM_VO_HdmiStartByDyRg(enIntfSync, enDstDyRg);
    }

    /******************************
    * Start mipi_tx device.
    * Note : do this after vo device started.
    ********************************/
    if(VO_INTF_MIPI == enVoIntfType)
    {
        SAMPLE_COMM_VO_StartMipiTx(enIntfSync);
    }

    return HI_SUCCESS;
}


HI_S32 SEEMMO_COMM_VO_StopVO(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
    VO_DEV                VoDev     = 0;
    VO_LAYER              VoLayer   = 0;
    SAMPLE_VO_MODE_E      enVoMode  = VO_MODE_BUTT;

    if (NULL == pstVoConfig)
    {
        SAMPLE_PRT("Error:argument can not be NULL\n");
        return HI_FAILURE;
    }

    VoDev     = pstVoConfig->VoDev;
    VoLayer   = pstVoConfig->VoDev;
    enVoMode  = pstVoConfig->enVoMode;

    if(VO_INTF_HDMI == pstVoConfig->enVoIntfType)
    {
        SAMPLE_COMM_VO_HdmiStop();
    }
    SEEMMO_COMM_VO_StopChn(VoLayer, enVoMode);
    SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);

    return HI_SUCCESS;
}


HI_S32 SEEMMO_COMM_VO_StopChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode)
{
    HI_S32 i;
    HI_S32 s32Ret    = HI_SUCCESS;
    HI_U32 u32WndNum = 0;

    switch (enMode)
    {
        case VO_MODE_1MUX:
        {
            u32WndNum = 1;
            break;
        }
        case VO_MODE_2MUX:
        {
            u32WndNum = 2;
            break;
        }
        case VO_MODE_4MUX:
        {
            u32WndNum = 4;
            break;
        }
        case VO_MODE_8MUX:
        {
            u32WndNum = 8;
            break;
        }
        case VO_MODE_9MUX:
        {
            u32WndNum = 9;
            break;
        }
        case VO_MODE_16MUX:
        {
            u32WndNum = 16;
            break;
        }
        default:
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
    }


    for (i = 0; i < u32WndNum; i++)
    {
        s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return s32Ret;
}


HI_S32 SEEMMO_COMM_VO_RestartVO(SEEMMO_LAYOUT_MODE_E enLayoutMode,SAMPLE_VO_MODE_E *enVoMode)
{
    HI_S32 s32Ret;
    VO_LAYER VoLayer = 0;
    //SAMPLE_VO_MODE_E enVoMode = VO_MODE_8MUX;
	
    //s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, *enVoMode);
    s32Ret = SEEMMO_COMM_VO_StopChn(VoLayer, *enVoMode);
    
//    s32Ret = SAMPLE_COMM_VO_StopVO(pstVoConfig);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("SAMPLE_COMM_VO_StopVO failed!\n");
        return s32Ret;
    }

    usleep(200000);
    switch (enLayoutMode)
    {
        case SEEMMO_LAYOUT_GENARAL:
        case SEEMMO_LAYOUT_LOOP:
            *enVoMode = VO_MODE_9MUX;
            break;
        case SEEMMO_LAYOUT_7_1:
            *enVoMode = VO_MODE_8MUX;
            break;
        default:
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
    }
    s32Ret = SEEMMO_COMM_VO_StartChn(VoLayer, *enVoMode, enLayoutMode);
//    s32Ret = SEEMMO_COMM_VO_StartVO(pstVoConfig, enLayoutMode);
    if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("SEEMMO_COMM_VO_StartVO failed!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
