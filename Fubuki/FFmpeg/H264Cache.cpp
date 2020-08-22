#include "H264Cache.h"

using namespace std;
using namespace OnePunchMan;

const int H264Cache::MaxOutputCount = 4;

const int H264Cache::Gop = 12;

const int H264Cache::FrameSize = 200 * 1024;

H264Cache::H264Cache(int channelIndex)
	:_channelIndex(channelIndex), _iFrameIndex(0), _pFrameIndex(0), _frameCount(0)
	, _extradata(new unsigned char[100]), _spsSize(0), _ppsSize(0), _rtmpOutput("rtmp://192.168.1.65:1935/live/9",-1)
{
	for (int i = 0; i < Gop; ++i)
	{
		FrameItem Item;
		Item.Data = new unsigned char[FrameSize];
		Item.Size = 0;
		_frameCache.push_back(Item);
	}
	_avParameters.codec_type = AVMEDIA_TYPE_VIDEO;
	_avParameters.codec_id = AV_CODEC_ID_H264;
	_avParameters.codec_tag = 0;
	_avParameters.format = 0;
	_avParameters.bit_rate = 0;
	_avParameters.bits_per_coded_sample = 0;
	_avParameters.bits_per_raw_sample = 8;
	_avParameters.profile = 66;
	_avParameters.level = 51;
	_avParameters.width = 1920;
	_avParameters.height = 1080;
	_avParameters.sample_aspect_ratio.num = 0;
	_avParameters.sample_aspect_ratio.den = 1;
	_avParameters.field_order = AV_FIELD_PROGRESSIVE;
	_avParameters.color_range = AVCOL_RANGE_UNSPECIFIED;
	_avParameters.color_primaries = AVCOL_PRI_UNSPECIFIED;
	_avParameters.color_trc = AVCOL_TRC_UNSPECIFIED;
	_avParameters.color_space = AVCOL_SPC_UNSPECIFIED;
	_avParameters.chroma_location = AVCHROMA_LOC_LEFT;
	_avParameters.video_delay = 0;
	_avParameters.channel_layout = 0;
	_avParameters.channels = 0;
	_avParameters.sample_rate = 0;
	_avParameters.block_align = 0;
	_avParameters.frame_size = 0;
	_avParameters.initial_padding = 0;
	_avParameters.trailing_padding = 0;
	_avParameters.seek_preroll = 0;
	_avParameters.extradata_size = 0;
	_avParameters.extradata = _extradata;

}

H264Cache::~H264Cache()
{
	for (unsigned int i = 0; i < _frameCache.size(); ++i)
	{
		delete[] _frameCache[i].Data;
	}
	delete[] _extradata;
	
}

bool H264Cache::AddOutputUrl(const std::string& outputUrl, int iFrameCount)
{
	if (_spsSize == 0 || _ppsSize == 0)
	{
		return false;
	}
	lock_guard<mutex> lck(_mutex);
	if (_outputItems.size() >= MaxOutputCount)
	{
		LogPool::Information("输出视频超过最大数量:", MaxOutputCount);
		return false;
	}
	if (_outputItems.find(outputUrl) == _outputItems.end())
	{
		Mp4Output* output=new Mp4Output(outputUrl, iFrameCount);
		if (output->Init(&_avParameters))
		{
			LogPool::Information("添加输出视频:", outputUrl, "当前缓存帧数量:", _frameCount);
			AVPacket* packet = av_packet_alloc();
			packet->pos = -1;
			for (unsigned int i = 0; i < _frameCount; ++i)
			{
				unsigned char* temp = (unsigned char*)av_malloc(_frameCache[i].Size);
				memcpy(temp, _frameCache[i].Data, _frameCache[i].Size);
				av_packet_from_data(packet, temp, _frameCache[i].Size);
				int frameType = i == 0 ? 5 : 1;
				packet->flags = frameType == 5 ? 1 : 0;
				output->WritePacket(packet, frameType);
				av_packet_unref(packet);
			}
			av_packet_free(&packet);
			_outputItems.insert(pair<string, Mp4Output*>(outputUrl, output));
		}
		else
		{
			LogPool::Information("添加输出视频失败:", outputUrl);
		}
	}
	return true;
}

bool H264Cache::OutputFinished(const std::string& outputUrl)
{
	lock_guard<mutex> lck(_mutex);
	map<string, Mp4Output*>::iterator it = _outputItems.find(outputUrl);
	if (it == _outputItems.end())
	{
		return true;
	}
	else
	{
		if (it->second->Finished())
		{
			LogPool::Information("结束输出视频:", it->first);
			delete it->second;
			_outputItems.erase(it);
			return true;
		}
		else
		{
			return false;
		}
	}
}

void H264Cache::PushPacket(unsigned char* data, int size)
{
	int frameType = 0;
	if (size >= 5)
	{
		frameType = data[4] & 0x1f;
		if (frameType == 7)
		{
			if (_spsSize == 0)
			{
				LogPool::Information("获取到sps数据:", _channelIndex);
				memcpy(_extradata, data, size);
				_spsSize = size;
				_avParameters.extradata_size += size;
			}
		}
		else if (frameType == 8)
		{
			if (_ppsSize == 0)
			{
				LogPool::Information("获取到pps数据:", _channelIndex);
				memcpy(_extradata + _spsSize, data, size);
				_ppsSize = size;
				_avParameters.extradata_size += size;

				bool b = _rtmpOutput.Init(&_avParameters);
				LogPool::Information("初始化rtmp", b);

			}
		}
		else if (frameType == 5)
		{
			//LogPool::Information("获取到I数据:", _channelIndex);
			if (WriteCache(data, size, 0, frameType))
			{
				_frameCount = 1;
			}
			else
			{
				_frameCount = 0;
			}
		}
		else if (frameType == 1&&_frameCount!=0)
		{
			//LogPool::Information("获取到P数据:", _channelIndex);
			if (WriteCache(data, size, _frameCount, frameType))
			{
				_frameCount += 1;
			}	
		}
	}
}

bool H264Cache::WriteCache(unsigned char* data, int size, int frameIndex,int frameType)
{
	AVPacket* packet1 = av_packet_alloc();
	unsigned char* temp1 = (unsigned char*)av_malloc(size);
	memcpy(temp1, data, size);
	av_packet_from_data(packet1, temp1, size);
	packet1->flags = frameType == 5 ? 1 : 0;
	packet1->pos = -1;
	_rtmpOutput.WritePacket(packet1, frameType);
	av_packet_unref(packet1);
	av_packet_free(&packet1);

	if (frameIndex >= 0
		&& frameIndex < static_cast<int>(_frameCache.size())
		&& FrameSize >= size)
	{
		lock_guard<mutex> lck(_mutex);
		memcpy(_frameCache[frameIndex].Data, data, size);
		if (!_outputItems.empty())
		{
			AVPacket* packet = av_packet_alloc();
			unsigned char* temp = (unsigned char*)av_malloc(size);
			memcpy(temp, data, size);
			av_packet_from_data(packet, temp, size);
			packet->flags = frameType == 5 ? 1 : 0;
			packet->pos = -1;
			for (map<string, Mp4Output*>::iterator it = _outputItems.begin(); it != _outputItems.end();++it)
			{
				it->second->WritePacket(packet, frameType);
			}
			av_packet_unref(packet);
			av_packet_free(&packet);
		
		}
		return true;
	}
	else
	{
		return false;
	}
}

void H264Cache::ClearCache()
{
	lock_guard<mutex> lck(_mutex);
	for (unsigned int i = 0; i < _frameCache.size(); ++i)
	{
		_frameCache[i].Size = 0;
	}
}
