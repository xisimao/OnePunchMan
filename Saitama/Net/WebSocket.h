#pragma once
#include "EndPoint.h"
#include "StringFormatter.h"
#include "Security.h"
#include "HttpHandler.h"

namespace OnePunchMan
{
	//websocket协议
	class WebSocket
	{
	public:

		/**
		* ws客户端连接协议
		* @param endPoint 连接到的地址
		* @param url 连接到的url
		* @return 返回协议字节流
		*/
		static std::string Connect(EndPoint endPoint, const std::string& url);

		/**
		* ws客户端主动关闭
		* @return 返回协议字节流
		*/
		static std::string Close();

		/**
		* ws服务端连接反馈协议
		* @param key 对方发送来的Key
		* @param code 响应代码
		* @return 返回协议字节流
		*/
		static std::string ConnectBack(const std::string& key, HttpCode code);

		/**
		* 获取ws推送字符串
		* @param value 要推送的字符串
		* @return ws推送字符串字节流
		*/
		static std::string Push(const std::string& value);

	};
}


