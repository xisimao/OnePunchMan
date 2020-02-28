#pragma once
#include "SocketHandler.h"
#include "Observable.h"

namespace Saitama
{
	enum class HttpCode
	{
		SwitchingProtocols=101,
		OK=200,
		NotFound=404,
		InternalServerError=500,
	};

	//http请求事件参数
	class HttpEventArgs
	{
	public:	
		//套接字
		int Socket;
		//请求url
		std::string Url;
		//请求内容
		std::string Request;
		//响应代码
		HttpCode Code;
		//响应内容
		std::string Response;
	};

	//http操作
	class HttpHandler: public SocketHandler
	{
	public:

		//http请求事件
		Observable<HttpEventArgs> Requested;

	protected:

		SocketHandler* CloneCore();

		ProtocolPacket HandleCore(int socket, unsigned int ip, unsigned short port, std::string::const_iterator begin, std::string::const_iterator end);


	};


}
