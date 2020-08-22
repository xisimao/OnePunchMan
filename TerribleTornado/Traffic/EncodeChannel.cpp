#include "EncodeChannel.h"

using namespace std;
using namespace OnePunchMan;

EncodeChannel::EncodeChannel(int encodeCount)
    :ThreadObject("encode"), _encodeCount(encodeCount)
{
    for (int i = 0; i < encodeCount; ++i)
    {
        _cache.push_back(new H264Cache(i + 1));
    }
}

bool EncodeChannel::InitHisi(int encodeCount, int width, int height)
{
#ifndef _WIN32
    //venc
    SIZE_S stDispSize;
    stDispSize.u32Width = width;
    stDispSize.u32Height = height;
    VENC_GOP_ATTR_S stGopAttr;

    memset(&stGopAttr, 0, sizeof(VENC_GOP_ATTR_S));

    stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
    stGopAttr.stNormalP.s32IPQpDelta = 3;
    for (int i = 0; i < encodeCount; i++)
    {
        HI_S32 s32Ret;


        /******************************************
         step 1:  Creat Encode Chnl
        ******************************************/

        VENC_CHN_ATTR_S        stVencChnAttr;
        HI_U32                 u32FrameRate = 25;
        HI_U32                 u32StatTime;
        HI_U32                 u32Gop = H264Cache::Gop;

        /******************************************
         step 1:  Create Venc Channel
        ******************************************/
        stVencChnAttr.stVencAttr.enType = PT_H264;
        stVencChnAttr.stVencAttr.u32MaxPicWidth = stDispSize.u32Width;
        stVencChnAttr.stVencAttr.u32MaxPicHeight = stDispSize.u32Height;
        stVencChnAttr.stVencAttr.u32PicWidth = stDispSize.u32Width;/*the picture width*/
        stVencChnAttr.stVencAttr.u32PicHeight = stDispSize.u32Height;/*the picture height*/
        stVencChnAttr.stVencAttr.u32BufSize = stDispSize.u32Width * stDispSize.u32Height * 2;/*stream buffer size*/
        stVencChnAttr.stVencAttr.u32Profile = 0;
        stVencChnAttr.stVencAttr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/

        if (VENC_GOPMODE_ADVSMARTP == stGopAttr.enGopMode)
        {
            u32StatTime = stGopAttr.stAdvSmartP.u32BgInterval / u32Gop;
        }
        else if (VENC_GOPMODE_SMARTP == stGopAttr.enGopMode)
        {
            u32StatTime = stGopAttr.stSmartP.u32BgInterval / u32Gop;
        }
        else
        {
            u32StatTime = 1;
        }

        stVencChnAttr.stVencAttr.stAttrH264e.bRcnRefShareBuf = HI_FALSE;
        VENC_H264_CBR_S    stH264Cbr;

        stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        stH264Cbr.u32Gop = u32Gop; /*the interval of IFrame*/
        stH264Cbr.u32StatTime = u32StatTime; /* stream rate statics time(s) */
        stH264Cbr.u32SrcFrameRate = u32FrameRate; /* input (vi) frame rate */
        stH264Cbr.fr32DstFrameRate = u32FrameRate; /* target frame rate */
        stH264Cbr.u32BitRate = 1024 * 2 + 2048;

        memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(VENC_H264_CBR_S));

        memcpy(&stVencChnAttr.stGopAttr, &stGopAttr, sizeof(VENC_GOP_ATTR_S));


        s32Ret = HI_MPI_VENC_CreateChn(i, &stVencChnAttr);
        if (HI_SUCCESS != s32Ret)
        {
            return false;
        }

        /******************************************
         step 2:  Start Recv Venc Pictures
        ******************************************/
        VENC_RECV_PIC_PARAM_S  stRecvParam;
        stRecvParam.s32RecvPicNum = -1;
        s32Ret = HI_MPI_VENC_StartRecvFrame(i, &stRecvParam);
        if (HI_SUCCESS != s32Ret)
        {
            return false;
        }
    }
#endif
    LogPool::Information(LogEvent::Decode, "init hisi encode");
    return true;
}

void EncodeChannel::UninitHisi(int encodeCount)
{
#ifndef _WIN32
    //venc
    for (int i = 0; i < encodeCount; ++i)
    {
        /******************************************
         step 1:  Stop Recv Pictures
        ******************************************/
        HI_MPI_VENC_StopRecvFrame(i);
        /******************************************
         step 2:  Distroy Venc Channel
        ******************************************/
        HI_MPI_VENC_DestroyChn(i);
    }
#endif // !_WIN32
    LogPool::Information(LogEvent::Decode, "uninit hisi encode");
}

bool EncodeChannel::AddOutput(int channelIndex, const std::string& outputUrl, int frameCount)
{
    if (channelIndex >= 1 && channelIndex <= _encodeCount)
    {
        return _cache[channelIndex - 1]->AddOutputUrl(outputUrl, frameCount);
    }
    else
    {
        LogPool::Warning(LogEvent::Encode, "添加视频输出出错,视频序号：", channelIndex);
        return false;
    }
}

bool EncodeChannel::OutputFinished(int channelIndex, const std::string& outputUrl)
{
    if (channelIndex >= 1 && channelIndex <= _encodeCount)
    {
        return _cache[channelIndex - 1]->OutputFinished(outputUrl);
    }
    else
    {
        LogPool::Warning(LogEvent::Encode, "查询视频输出结果出错,视频序号：", channelIndex);
        return true;
    }
}

#ifndef _WIN32
void EncodeChannel::PushFrame(int channelIndex, VIDEO_FRAME_INFO_S* frame)
{
    if (channelIndex >= 1 && channelIndex <= _encodeCount)
    {
        HI_MPI_VENC_SendFrame(channelIndex - 1, frame, 0);
    }
}
#endif // !_WIN32

void EncodeChannel::StartCore()
{
#ifndef _WIN32
    HI_S32 maxfd = 0;
    struct timeval timeoutVal;
    fd_set read_fds;
    HI_S32 vencFds[VENC_MAX_CHN_NUM];

    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    VENC_STREAM_BUF_INFO_S stStreamBufInfo[VENC_MAX_CHN_NUM];

    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    for (int encodeIndex = 0; encodeIndex < _encodeCount; encodeIndex++)
    {
        /* Set Venc Fd. */
        vencFds[encodeIndex] = HI_MPI_VENC_GetFd(encodeIndex);
        if (vencFds[encodeIndex] < 0)
        {
            LogPool::Error(LogEvent::Encode, "HI_MPI_VENC_GetFd");
            return;
        }
        if (maxfd <= vencFds[encodeIndex])
        {
            maxfd = vencFds[encodeIndex];
        }

        s32Ret = HI_MPI_VENC_GetStreamBufInfo(encodeIndex, &stStreamBufInfo[encodeIndex]);
        if (HI_SUCCESS != s32Ret)
        {
            LogPool::Error(LogEvent::Encode, "HI_MPI_VENC_GetStreamBufInfo");
            return;
        }
    }

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (!_cancelled)
    {
        FD_ZERO(&read_fds);
        for (int encodeIndex = 0; encodeIndex < _encodeCount; encodeIndex++)
        {
            FD_SET(vencFds[encodeIndex], &read_fds);
        }

        timeoutVal.tv_sec = 0;
        timeoutVal.tv_usec = 10000;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &timeoutVal);
        if (s32Ret < 0)
        {
            LogPool::Error(LogEvent::Encode, "select", s32Ret);
            break;
        }
        else if (s32Ret == 0)
        {
            continue;
        }
        else
        {

            for (int encodeIndex = 0; encodeIndex < _encodeCount; encodeIndex++)
            {
                if (FD_ISSET(vencFds[encodeIndex], &read_fds))
                {

                    /*******************************************************
                     step 2.1 : query how many packs in one-frame stream.
                    *******************************************************/
                    memset(&stStream, 0, sizeof(stStream));

                    s32Ret = HI_MPI_VENC_QueryStatus(encodeIndex, &stStat);
                    if (HI_SUCCESS != s32Ret)
                    {
                        LogPool::Error(LogEvent::Encode, "HI_MPI_VENC_QueryStatus");
                        break;
                    }
                    /*******************************************************
                    step 2.2 :suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
                    *******************************************************/
                    if (0 == stStat.u32CurPacks)
                    {
                        continue;
                    }
                    /*******************************************************
                     step 2.3 : malloc corresponding number of pack nodes.
                    *******************************************************/
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        LogPool::Error(LogEvent::Encode, "malloc VENC_PACK_S");
                        break;
                    }

                    /*******************************************************
                     step 2.4 : call mpi to get one-frame stream
                    *******************************************************/
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(encodeIndex, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        LogPool::Error(LogEvent::Encode, "HI_MPI_VENC_GetStream VENC_PACK_S");
                        break;
                    }

                    /*******************************************************
                     step 2.5 : save frame to file
                    *******************************************************/
                    for (HI_U32 packetIndex = 0; packetIndex < stStream.u32PackCount; packetIndex++)
                    {
                        _cache[encodeIndex]->PushPacket((unsigned char*)(stStream.pstPack[packetIndex].pu8Addr + stStream.pstPack[packetIndex].u32Offset), stStream.pstPack[packetIndex].u32Len - stStream.pstPack[packetIndex].u32Offset);
                    }
                    /*******************************************************
                     step 2.6 : release stream
                     *******************************************************/
                    s32Ret = HI_MPI_VENC_ReleaseStream(encodeIndex, &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        LogPool::Error(LogEvent::Encode, "HI_MPI_VENC_ReleaseStream");
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }

                    /*******************************************************
                     step 2.7 : free pack nodes
                    *******************************************************/
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;

                }
            }
        }
    }
#endif // _WIN32
}
