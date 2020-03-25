#include "HttpHandler.h"

using namespace std;
using namespace Saitama;

const string HttpFunction::Get="GET";
const string HttpFunction::Post = "POST";
const string HttpFunction::Put = "PUT";
const string HttpFunction::Delete = "DELETE";
const string HttpFunction::Options = "OPTIONS";

SocketHandler* HttpHandler::CloneCore()
{
	HttpHandler* handler= new HttpHandler();
	handler->HttpReceived.Copy(&this->HttpReceived);
	return handler;
}

std::string HttpHandler::BuildResponse(HttpCode code, const string& responseJson)
{
	stringstream ss;
	ss << "HTTP/1.1 " << static_cast<int>(code) << "\r\n"
		<< "Date: " << DateTime::UtcNow().ToString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n"
		<< "Server: cat\r\n"
		<< "Content-Type: application/json;charset=UTF-8\r\n"
		<< "Content-Length: " << responseJson.size() << "\r\n"
		<< "Access-Control-Allow-Origin: *\r\n"
		<< "\r\n"
		<< responseJson;
	return ss.str();
}

SocketHandler::ProtocolPacket HttpHandler::HandleCore(int socket, unsigned int ip, unsigned short port, string::const_iterator begin, string::const_iterator end)
{ 
	string httpProtocol(begin, end);
	vector<string> lines = StringEx::Split(httpProtocol, "\r\n");
	HttpReceivedEventArgs e;
	e.Socket = socket;
	if (lines.empty())
	{
		LogPool::Warning(LogEvent::Socket, "http empty", httpProtocol);
		return ProtocolPacket(AnalysisResult::Empty, 0, static_cast<unsigned int>(httpProtocol.size()), 0,0);
	}
	else
	{
		//第一行解析请求类型和url
		vector<string> datas = StringEx::Split(lines[0], " ", true);
		if (datas.size() <2)
		{
			LogPool::Warning(LogEvent::Socket, "first line empty", httpProtocol);
			return ProtocolPacket(AnalysisResult::Empty, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
		}
		string func = StringEx::ToUpper(datas[0]);
		e.Function = func;
		e.Url = datas[1];

		//用空行判断是否是全包
		size_t packetSize = lines[0].size() + 2;
		bool hasEmpty = false;
		for (unsigned int i=1; i < lines.size(); ++i)
		{
			packetSize += lines[i].size() + 2;
			if (lines[i].empty())
			{
				hasEmpty = true;
				break;
			}
		}

		if (!hasEmpty)
		{
			return ProtocolPacket(AnalysisResult::Half, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
		}

		if (func.compare(HttpFunction::Options) == 0)
		{
			stringstream ss;
			ss << "HTTP/1.1 204 No Content\r\n"
				<< "Date: " << DateTime::UtcNow().ToString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n"
				<< "Server: cat\r\n"
				<< "Access-Control-Allow-Headers: authorization,content-type\r\n"
				<< "Access-Control-Allow-Methods: GET,POST,DELETE,PUT\r\n"
				<< "Access-Control-Allow-Origin: *\r\n"
				<< "\r\n"
				<< "\r\n";
			SendTcp(socket, ss.str(), NULL);
			return ProtocolPacket(AnalysisResult::Request, 0, static_cast<unsigned int>(packetSize), 0, 0);
		}
		else if (func.compare(HttpFunction::Get) == 0
			|| func.compare(HttpFunction::Delete) == 0)
		{
			HttpReceived.Notice(&e);
			string response = BuildResponse(HttpCode::OK, e.ResponseJson);
			SendTcp(socket, response, NULL);
			return ProtocolPacket(AnalysisResult::Request, 0, static_cast<unsigned int>(packetSize), 0, 0);
		}
		else if (func.compare(HttpFunction::Post) == 0
			|| func.compare(HttpFunction::Put) == 0)
		{
			//获取body长度
			int length = -1;
			for (unsigned int i=1; i < lines.size(); ++i)
			{
				vector<string> values = StringEx::Split(lines[i], ":", true);
				if (values.size() > 1 && StringEx::ToUpper(values[0]).compare("CONTENT-LENGTH") == 0)
				{
					length=StringEx::Convert<int>(values[1]);
					break;
				}
			}

			//没有长度
			if (length==-1)
			{
				LogPool::Warning(LogEvent::Socket, "not found CONTENT-LENGTH", httpProtocol);
				return ProtocolPacket(AnalysisResult::Empty, 0, static_cast<unsigned int>(packetSize), 0, 0);
			}
			else
			{
				//半包
				if (packetSize + length > httpProtocol.size())
				{
					return ProtocolPacket(AnalysisResult::Half, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
				}
				else
				{
					e.RequestJson = httpProtocol.substr(packetSize, length);
					HttpReceived.Notice(&e);
					string response = BuildResponse(HttpCode::OK, e.ResponseJson);
					SendTcp(socket, response, NULL);
					return ProtocolPacket(AnalysisResult::Request, 0, static_cast<unsigned int>(packetSize + length), 0, 0);
				}
			}
		}
		else
		{
			LogPool::Warning(LogEvent::Socket, "http error fun", httpProtocol);
			return ProtocolPacket(AnalysisResult::Empty, 0, static_cast<unsigned int>(packetSize), 0,0);
		}
	}
}