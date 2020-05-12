#include "SocketHandler.h"

using namespace std;
using namespace OnePunchMan;

SocketHandler::SocketHandler()
	:_transmitSize(0),_receiveSize(0)
{
	
}

SocketHandler* SocketHandler::Clone(int socket)
{
	SocketHandler* handler = CloneCore();
	return handler;
}

unsigned long long SocketHandler::TransmitSize()
{
	return _transmitSize;
}

unsigned long long SocketHandler::ReceiveSize()
{
	return _receiveSize;
}

SocketResult SocketHandler::SendTcp(int socket, const string& buffer, AsyncHandler* handler)
{
	_transmitSize += static_cast<unsigned int>(buffer.size());
	if (handler != NULL)
	{
		lock_guard<mutex> lck(_mutex);
		_handlers.push_back(handler);
	}
	//LogPool::Debug(LogEvent::Socket,socket, "-", buffer.size(), StringEx::ToHex(buffer.begin(),buffer.end()));
	return Socket::SendTcp(socket, buffer.c_str(), static_cast<unsigned int>(buffer.size())) ? SocketResult::Success : SocketResult::SendFailed;
}

SocketResult SocketHandler::SendUdp(int socket, const EndPoint& remoteEndPoint, const string& buffer, AsyncHandler* handler)
{
	_transmitSize += static_cast<unsigned int>(buffer.size());
	if (handler != NULL)
	{
		lock_guard<mutex> lck(_mutex);
		_handlers.push_back(handler);
	}
	//LogPool::Debug(LogEvent::Socket, socket, remoteEndPoint.ToString(), "-", buffer.size(), StringEx::ToHex(buffer.begin(), buffer.end()));
	return Socket::SendUdp(socket, remoteEndPoint, buffer.c_str(), static_cast<unsigned int>(buffer.size())) ? SocketResult::Success : SocketResult::SendFailed;;
}

void SocketHandler::Handle(int socket, unsigned int ip, unsigned short port, const char* buffer, unsigned int size)
{
	_receiveSize += size;
	//if (ip == 0 && port == 0)
	//{
	//	LogPool::Debug(LogEvent::Socket, socket,"+", size, StringEx::ToHex(buffer,size));
	//}
	//else
	//{
	//	LogPool::Debug(LogEvent::Socket, socket, EndPoint(ip, port).ToString(), "+", size, StringEx::ToHex(buffer,size));
	//}
	
	_residueBuffer.append(buffer,size);

	unsigned int offset = 0;
	do
	{
		ProtocolPacket packet = HandleCore(socket, ip, port, _residueBuffer.begin() + offset, _residueBuffer.end());
		if (packet.Result == AnalysisResult::Response)
		{
			if (!_handlers.empty())
			{
				lock_guard<mutex> lck(_mutex);

				for (list<AsyncHandler*>::iterator it = _handlers.begin(); it != _handlers.end();)
				{
					AsyncHandler* handler = *it;
					if (handler->IsCompleted())
					{
						it = _handlers.erase(it);
						delete handler;
					}
					else
					{
						if (handler->ProtocolId() == packet.ProtocolId&&handler->TimeStamp() == packet.TimeStamp)
						{
							handler->Handle(_residueBuffer.begin() + offset + packet.Offset, _residueBuffer.begin() + offset + packet.Offset+packet.Size);
						}
						++it;
					}
				}
			}
		}
		else if (packet.Result == AnalysisResult::Half)
		{
			_residueBuffer.assign(_residueBuffer.end() - packet.Size, _residueBuffer.end());
			return;
		}
		offset += packet.Offset + packet.Size;

	} while (offset < _residueBuffer.size());
	_residueBuffer.clear();
}
