#include "sample_comm.h"

#if __cplusplus
//extern "C"{
#endif

HI_S32 SEEMMO_COMM_VENC_StartGetStream(VENC_CHN VeChn[],HI_S32 s32Cnt);
HI_S32 SEEMMO_COMM_VENC_StopGetStream(void);
void SendFrameToVenc(int chn,VIDEO_FRAME_INFO_S *pstFrame);
HI_S32 SEEMMO_COMM_VENC_Start(VENC_CHN VencChn, PAYLOAD_TYPE_E enType,  PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode, HI_U32  u32Profile, VENC_GOP_ATTR_S *pstGopAttr);


void SEEMMO_VRC_Close(int index);
void SEEMMO_VRC_Create(int index,const char * sUrl);
void SEEMMO_VRC_ReCreate(int index,const char * sUrl);


#if __cplusplus
//}
#endif

