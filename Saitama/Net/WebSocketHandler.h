#pragma once
#include "SocketHandler.h"
#include "SocketChannel.h"
#include "Observable.h"
#include "WebSocket.h"

namespace OnePunchMan
{
	//客户端连接事件参数
	class WebSocketOpenedEventArgs
	{
	public:
		//客户端套接字
		int Socket;
		//连接url
		std::string Url;
		//响应代码
		HttpCode Code;
	};

	//websocket处理
	class WebSocketHandler : public SocketHandler
	{
	public:

		//客户端连接事件
		Observable<WebSocketOpenedEventArgs> Opened;

		//客户端主动断开事件
		Observable<ClosedEventArgs> Closed;

	protected:

		SocketHandler* CloneCore();

		ProtocolPacket HandleCore(int socket, unsigned int ip, unsigned short port, std::string::const_iterator begin, std::string::const_iterator end);

	};


}
