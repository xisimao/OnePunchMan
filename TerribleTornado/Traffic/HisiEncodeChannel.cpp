#include "HisiEncodeChannel.h"

using namespace std;
using namespace OnePunchMan;

HisiEncodeChannel::HisiEncodeChannel(int videoCount)
    :ThreadObject("encode"), _videoCount(videoCount)
{
    for (int i = 0; i < videoCount; ++i)
    {
        _outputHandlers.push_back(NULL);
    }
}

int HisiEncodeChannel::StartEncode(int channelIndex,const std::string& outputUrl, const std::string& inputUrl,int frameCount)
{
    lock_guard<mutex> lck(_mutex);
    for (unsigned int i = 0; i < _outputHandlers.size(); ++i)
    {
        if (_outputHandlers[i]==NULL)
        {
            OutputHandler* handler = new OutputHandler();
            handler->Init(channelIndex,outputUrl, inputUrl,frameCount);
            _outputHandlers[i] = handler;
            return static_cast<int>(i);
        }
    }
    return -1;
}

#ifndef _WIN32
void HisiEncodeChannel::PushFrame(int channelIndex, VIDEO_FRAME_INFO_S* frame)
{
    lock_guard<mutex> lck(_mutex);
    for (unsigned int i = 0; i < _outputHandlers.size(); ++i)
    {
        if (_outputHandlers[i] != NULL
            && _outputHandlers[i]->ChannelIndex()==channelIndex
            && !_outputHandlers[i]->Finished())
        {
            HI_MPI_VENC_SendFrame(i, frame, 0);
        }
    }
}
#endif // !_WIN32

bool HisiEncodeChannel::Finished(int index)
{
    lock_guard<mutex> lck(_mutex);
    if (index >= 0 
        && index < static_cast<int>(_outputHandlers.size())
        && _outputHandlers[index] != NULL)
    {
        return _outputHandlers[index]->Finished();
    }
    return true;
}

void HisiEncodeChannel::StopEncode(int index)
{
    lock_guard<mutex> lck(_mutex);
    if (index >= 0
        && index < static_cast<int>(_outputHandlers.size())
        && _outputHandlers[index] != NULL)
    {
        _outputHandlers[index]->Uninit();
        delete _outputHandlers[index];
        _outputHandlers[index] = NULL;
    }
}

void HisiEncodeChannel::StartCore()
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

    for (int videoIndex = 0; videoIndex < _videoCount; videoIndex++)
    {
        /* Set Venc Fd. */
        vencFds[videoIndex] = HI_MPI_VENC_GetFd(videoIndex);
        if (vencFds[videoIndex] < 0)
        {
            LogPool::Error(LogEvent::Encode, "HI_MPI_VENC_GetFd");
            return;
        }
        if (maxfd <= vencFds[videoIndex])
        {
            maxfd = vencFds[videoIndex];
        }

        s32Ret = HI_MPI_VENC_GetStreamBufInfo(videoIndex, &stStreamBufInfo[videoIndex]);
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
        for (int videoIndex = 0; videoIndex < _videoCount; videoIndex++)
        {
            FD_SET(vencFds[videoIndex], &read_fds);
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

            for (int videoIndex = 0; videoIndex < _videoCount; videoIndex++)
            {
                if (FD_ISSET(vencFds[videoIndex], &read_fds))
                {

                    /*******************************************************
                     step 2.1 : query how many packs in one-frame stream.
                    *******************************************************/
                    memset(&stStream, 0, sizeof(stStream));

                    s32Ret = HI_MPI_VENC_QueryStatus(videoIndex, &stStat);
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
                    s32Ret = HI_MPI_VENC_GetStream(videoIndex, &stStream, HI_TRUE);
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
                    unique_lock<mutex> lck(_mutex);
                    for (HI_U32 packetIndex = 0; packetIndex < stStream.u32PackCount; packetIndex++) 
                    {
                        if (_outputHandlers[videoIndex] != NULL)
                        {
                            _outputHandlers[videoIndex]->PushPacket((unsigned char*)(stStream.pstPack[packetIndex].pu8Addr + stStream.pstPack[packetIndex].u32Offset), stStream.pstPack[packetIndex].u32Len - stStream.pstPack[packetIndex].u32Offset);
                        }
                    }
                    lck.unlock();
                    /*******************************************************
                     step 2.6 : release stream
                     *******************************************************/
                    s32Ret = HI_MPI_VENC_ReleaseStream(videoIndex, &stStream);
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
