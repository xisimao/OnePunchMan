#include "WebSocket.h"

using namespace std;
using namespace Saitama;

string WebSocket::Connect(EndPoint endPoint, const std::string& url)
{
	stringstream ss;
	ss << "GET "<<url<<" HTTP/1.1\r\n"
		<< "Host: "<<endPoint.ToString()<<"\r\n"
		<< "Connection: Upgrade\r\n"
		<< "Upgrade: websocket\r\n"
		<< "Origin: Kakegurui\r\n"
		<< "Accept-Encoding: utf-8\r\n"
		<< "Sec-WebSocket-Version: 13\r\n"
		<< "Sec-WebSocket-Key: Z1+x2Q9e1fuIVI6MG2uX4w==\r\n"
		<< "\r\n";
	return ss.str();
}

string WebSocket::Close()
{
	string buffer;
	buffer.push_back((char)0x88);
	buffer.push_back((char)0x00);
	return buffer;
}

string WebSocket::ConnectBack(const std::string& key, HttpCode code)
{
	unsigned char encryptedKey[20] = { 0 };
	Security::Sha1(key, encryptedKey,20);
	string encryptedString = Security::Base64_Encode(encryptedKey, 20);
	stringstream ss;
	ss << "HTTP/1.1 "<<static_cast<int>(code)<<" \r\n";
	ss << "Upgrade: websocket\r\n";
	ss << "Connection: upgrade\r\n";
	ss << "Sec-WebSocket-Accept: " << encryptedString << "\r\n";
	ss << "Sec-WebSocket-Extensions: permessage-deflate;client_max_window_bits=15\r\n";
	ss << "Date: " << DateTime::UtcNow().ToString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	ss << "\r\n";
	return ss.str();
}

string WebSocket::Push(const std::string& value)
{
	string buffer;
	buffer.reserve(value.size() + 10);
	buffer.push_back((char)0x81);
	if (value.size() > (std::numeric_limits<unsigned short>::max)())
	{
		buffer.push_back((char)127);
		StringFormatter::Serialize(&buffer, static_cast<unsigned long long>(value.size()));

	}
	else if (value.size() > 125)
	{
		buffer.push_back((char)126);
		StringFormatter::Serialize(&buffer, static_cast<unsigned short>(value.size()));

	}
	else
	{
		buffer.push_back((char)value.size());
	}
	buffer.append(value);
	return buffer;
}