#pragma once
#include "SocketHandler.h"
#include "Observable.h"

namespace Saitama
{
	enum class HttpCode
	{
		SwitchingProtocols=101,
		OK=200,
		BadRequest=400,
		NotFound=404,
		InternalServerError=500,
	};

	class HttpFunction
	{
	public:
		static const std::string Get;
		static const std::string Post;
		static const std::string Put;
		static const std::string Delete;
		static const std::string Options;
	};

	//http请求事件参数
	class HttpReceivedEventArgs
	{
	public:	
		//套接字
		int Socket;
		//请求方法
		std::string Function;
		//请求url
		std::string Url;
		//请求内容
		std::string RequestJson;
		//响应代码
		HttpCode Code;
		//响应内容
		std::string ResponseJson;
	};

	//http操作
	class HttpHandler: public SocketHandler
	{
	public:

		//接收到http消息事件
		Observable<HttpReceivedEventArgs> HttpReceived;

	protected:

		SocketHandler* CloneCore();

		ProtocolPacket HandleCore(int socket, unsigned int ip, unsigned short port, std::string::const_iterator begin, std::string::const_iterator end);

	private:

		/**
		* @brief: 拼组响应报文
		* @param: code 响应http编码
		* @param: origin 跨域
		* @param: responseJson 响应json字符串
		* @return: 响应报文
		*/
		std::string BuildResponse(HttpCode code, const std::string& origin, const std::string& responseJson);

	};


}
