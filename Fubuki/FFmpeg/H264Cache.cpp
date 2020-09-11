#include "H264Cache.h"

using namespace std;
using namespace OnePunchMan;

const int H264Cache::MaxOutputCount = 4;

const int H264Cache::MaxExtraDataSize = 100;

const int H264Cache::Gop = 12;

const int H264Cache::FrameSize = 200 * 1024;

H264Cache::H264Cache(int channelIndex)
	:_channelIndex(channelIndex), _iFrameIndex(0), _pFrameIndex(0), _frameCount(0)
	, _extraData(new unsigned char[MaxExtraDataSize]),_spsSize(0), _ppsSize(0)
{
	for (int i = 0; i < Gop; ++i)
	{
		FrameItem Item;
		Item.Data = new unsigned char[FrameSize];
		Item.Size = 0;
		_frameCache.push_back(Item);
	}
}

H264Cache::~H264Cache()
{
	for (unsigned int i = 0; i < _frameCache.size(); ++i)
	{
		delete[] _frameCache[i].Data;
	}
	delete[] _extraData;
}

bool H264Cache::AddOutputUrl(const std::string& outputUrl, int iFrameCount)
{
	if (_spsSize==0 || _ppsSize==0)
	{
		LogPool::Information(LogEvent::Encode,"not found sps or pps data. sps:", _spsSize,"pps:", _ppsSize, outputUrl);
		return false;
	}
	lock_guard<mutex> lck(_mutex);
	if (_outputItems.size() >= MaxOutputCount)
	{
		LogPool::Information(LogEvent::Encode, "output files over max config,output file:", outputUrl," config:", MaxOutputCount);
		return false;
	}
	if (_outputItems.find(outputUrl) == _outputItems.end())
	{
		FFmpegOutput* output=new FFmpegOutput();
		if (output->Init(outputUrl, iFrameCount, _extraData, _spsSize+_ppsSize))
		{
			for (unsigned int i = 0; i < _frameCount; ++i)
			{
				output->WritePacket(_frameCache[i].Data, _frameCache[i].Size, i==0? FrameType::I: FrameType::P);
			}
			_outputItems.insert(pair<string, FFmpegOutput*>(outputUrl, output));
		}
	}
	else
	{
		LogPool::Warning(LogEvent::Encode, "output file alread existed:", outputUrl);
	}
	return true;
}

void H264Cache::RemoveOutputUrl(const std::string& outputUrl)
{
	lock_guard<mutex> lck(_mutex);
	map<string, FFmpegOutput*>::iterator it = _outputItems.find(outputUrl);
	if (it != _outputItems.end())
	{
		LogPool::Information(LogEvent::Encode, "remove output file:", it->first);
		delete it->second;
		_outputItems.erase(it);
	}
}

bool H264Cache::OutputFinished(const std::string& outputUrl)
{
	lock_guard<mutex> lck(_mutex);
	map<string, FFmpegOutput*>::iterator it = _outputItems.find(outputUrl);
	if (it == _outputItems.end())
	{
		LogPool::Information(LogEvent::Encode, "not found check finish output file:", outputUrl);
		return true;
	}
	else
	{
		if (it->second->Finished())
		{
			LogPool::Information(LogEvent::Encode, "aotu remove out file when check finish:", outputUrl);
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
			if (_spsSize==0)
			{
				if (size+ _ppsSize > MaxExtraDataSize)
				{
					LogPool::Error("sps too large,channel index:",_channelIndex, "sps length:", size, "pps length:", _ppsSize);
					return;
				}
				LogPool::Information(LogEvent::Encode,"got sps,channel index:", _channelIndex);
				memcpy(_extraData, data, size);
				_spsSize = size;
			}
		}
		else if (frameType == 8)
		{
			if (_ppsSize==0)
			{
				if (_spsSize+size > MaxExtraDataSize)
				{
					LogPool::Error("pps too large,channel index:", _channelIndex, "sps length:", _spsSize, "pps length:", size);
					return;
				}
				LogPool::Information(LogEvent::Encode, "got pps,channel index:", _channelIndex);
				memcpy(_extraData + _spsSize, data, size);
				_ppsSize = size;
			}
		}
		else if (frameType == 5)
		{
			LogPool::Debug(LogEvent::Encode, "获取到I数据:", _channelIndex);
			if (WriteCache(data, size, 0, FrameType::I))
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
			LogPool::Debug(LogEvent::Encode, "获取到P数据:", _channelIndex);
			if (WriteCache(data, size, _frameCount, FrameType::P))
			{
				_frameCount += 1;
			}	
		}
	}
}

bool H264Cache::WriteCache(unsigned char* data, int size, int frameIndex,FrameType frameType)
{
	if (frameIndex >= 0
		&& frameIndex < static_cast<int>(_frameCache.size())
		&& FrameSize >= size)
	{
		lock_guard<mutex> lck(_mutex);
		memcpy(_frameCache[frameIndex].Data, data, size);
		if (!_outputItems.empty())
		{
			for (map<string, FFmpegOutput*>::iterator it = _outputItems.begin(); it != _outputItems.end(); ++it)
			{
				it->second->WritePacket(data,size, frameType);
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}
