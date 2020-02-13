#include "AsyncQueue.h"

using namespace std;
using namespace Saitama;

const int AsyncQueue::Capacity = 1 * 1024 * 1024;

const int AsyncQueue::HeadSize = 4;

AsyncQueue::AsyncQueue()
{
	//实际上容量多一个字节，避免出现pushIndex和popIndex相等时，可能是容器为空
	//或者是容器满的二义性判断
	_buffer = new char[Capacity+1];
	_popIndex = 0;
	_pushIndex = 0;
}

AsyncQueue::~AsyncQueue()
{
	delete[] _buffer;
}

unsigned int AsyncQueue::Size()
{
	return _pushIndex >= _popIndex ? _pushIndex - _popIndex : _pushIndex + Capacity + 1 - _popIndex;
}

bool AsyncQueue::Empty()
{
	return _pushIndex == _popIndex;
}

bool AsyncQueue::NeedTurn(unsigned int index,unsigned int size)
{
	return index+size > Capacity;
}

unsigned int AsyncQueue::NewIndex(unsigned int index,unsigned int size)
{
	return  index + size>Capacity ? size - (Capacity + 1 - index) : index + size;
}

bool AsyncQueue::Push(const char* buffer, unsigned int size)
{
	if (size == 0)
	{
		return false;
	}

	lock_guard<mutex> lck(_pushMutex);
	unsigned int queueSize = Size();
	if (HeadSize + size + queueSize> Capacity)
	{
		return false;
	}

	//复制长度
	if (NeedTurn(_pushIndex,HeadSize))
	{
		unsigned int middle = Capacity + 1 - _pushIndex;
		PushSize(_buffer + _pushIndex, 0, middle, size);
		PushSize(_buffer, middle, HeadSize - middle, size);
	}
	else
	{
		PushSize(_buffer + _pushIndex, 0, HeadSize, size);
	}

	//复制数据区
	unsigned int tempPushIndex = NewIndex(_pushIndex, HeadSize);

	if (NeedTurn(tempPushIndex,size))
	{
		unsigned int middle = Capacity + 1 - tempPushIndex;
		memcpy(_buffer + tempPushIndex, buffer, middle);
		memcpy(_buffer, buffer + middle, size - middle);
	}
	else
	{
		memcpy(_buffer + tempPushIndex, buffer, size);
	}

	//修改相关变量，对于判断的关键需要在内存复制后在修改
	_pushIndex = NewIndex(tempPushIndex, size);
	return true;
}

int AsyncQueue::Pop(char* buffer, unsigned int capacity)
{
	lock_guard<mutex> lck(_popMutex);
	if (Size() == 0)
	{
		return 0;
	}

	//复制长度

	unsigned int size = 0;
	if (NeedTurn(_popIndex,HeadSize))
	{
		unsigned int middle = Capacity + 1 - _popIndex;
		PopSize(_buffer + _popIndex, 0, middle, &size);
		PopSize(_buffer, middle, HeadSize - middle, &size);
	}
	else
	{
		PopSize(_buffer + _popIndex, 0, HeadSize, &size);
	}

	unsigned int tempPopIndex = NewIndex(_popIndex, HeadSize);

	//复制数据
	if (size <= capacity)
	{
		if (NeedTurn(tempPopIndex, size))
		{
			unsigned int middle = Capacity + 1 - tempPopIndex;
			memcpy(buffer, _buffer + tempPopIndex, middle);
			memcpy(buffer + middle, _buffer, size - middle);
		}
		else
		{
			memcpy(buffer, _buffer + tempPopIndex, size);
		}
	}
	else
	{
		size = -1;
	}

	//修改变量
	_popIndex = NewIndex(tempPopIndex, size);
	return size;
}

void AsyncQueue::Show()
{
	if (_pushIndex == _popIndex)
	{
		cout << "empty" << endl;
		return;
	}

	for (unsigned int i = 0; i < Capacity + 1; ++i)
	{
		if (_pushIndex > _popIndex)
		{
			if (i < _pushIndex && i >= _popIndex)
			{
				std::cout << static_cast<int>(_buffer[i]) << " ";
			}
			else
			{
				std::cout << "0 ";
			}
		}
		else
		{
			if (i < _pushIndex || i >= _popIndex)
			{
				std::cout << static_cast<int>(_buffer[i]) << " ";
			}
			else
			{
				std::cout << "0 ";
			}
		}

	}
	std::cout << std::endl;
}
