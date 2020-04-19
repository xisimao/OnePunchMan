

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#if 0
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
#endif

#include "sample_comm.h"

typedef enum {
    SEEMMO_LAYOUT_GENARAL,
    SEEMMO_LAYOUT_7_1,
    SEEMMO_LAYOUT_LOOP,
} SEEMMO_LAYOUT_MODE_E;

HI_S32 SEEMMO_COMM_VO_StartChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode, SEEMMO_LAYOUT_MODE_E enLayoutMode);
HI_S32 SEEMMO_COMM_VO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig, SEEMMO_LAYOUT_MODE_E enLayoutMode);
HI_S32 SEEMMO_COMM_VO_RestartVO(SEEMMO_LAYOUT_MODE_E enLayoutMode,SAMPLE_VO_MODE_E *enVoMode);
HI_S32 SEEMMO_COMM_VO_StopVO(SAMPLE_VO_CONFIG_S *pstVoConfig);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
