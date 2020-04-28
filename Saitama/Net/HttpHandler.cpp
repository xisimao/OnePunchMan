#include "HttpHandler.h"

using namespace std;
using namespace OnePunchMan;

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
		e.Function = StringEx::ToUpper(datas[0]);
		e.Url = datas[1];
		//默认设置为404，如果后续没有处理则认为无效的url
		e.Code = HttpCode::NotFound;

		//用空行判断是否是全包
		size_t packetSize = lines[0].size() + 2;
		bool hasEmpty = false;
		int length = 0;
		for (unsigned int i=1; i < lines.size(); ++i)
		{
			packetSize += lines[i].size() + 2;

			vector<string> values = StringEx::Split(lines[i], ":", true);
			if (values.size() >= 2)
			{
				if (StringEx::ToUpper(values[0]).compare("CONTENT-LENGTH") == 0)
				{
					length = StringEx::Convert<int>(values[1]);
				}
				else if (StringEx::ToUpper(values[0]).compare("HOST") == 0)
				{
					e.Host = StringEx::Trim(values[1]);
				}
			}

			if (lines[i].empty())
			{
				hasEmpty = true;
				break;
			}
		}

		//半包
		if (!hasEmpty||packetSize + length > httpProtocol.size())
		{
			return ProtocolPacket(AnalysisResult::Half, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
		}

		string response;
		if (e.Function.compare(HttpFunction::Options) == 0)
		{
			stringstream ss;
			ss << "HTTP/1.1 204 No Content\r\n"
				<< "Date: " << DateTime::UtcNow().ToString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n"
				<< "Server: cat\r\n"
				<< "Access-Control-Allow-Headers: authorization,content-type\r\n"
				<< "Access-Control-Allow-Methods: GET,POST,DELETE,PUT\r\n"
				<< "Access-Control-Allow-Origin: *\r\n"
				<< "\r\n";
			response = ss.str();
		}
		else if (e.Function.compare(HttpFunction::Get) == 0
			|| e.Function.compare(HttpFunction::Delete) == 0)
		{
			HttpReceived.Notice(&e);
			response = BuildResponse(e.Code, e.ResponseJson);
		}
		else if (e.Function.compare(HttpFunction::Post) == 0
			|| e.Function.compare(HttpFunction::Put) == 0)
		{
			e.RequestJson = httpProtocol.substr(packetSize, length);
			HttpReceived.Notice(&e);
			response = BuildResponse(e.Code, e.ResponseJson);
		}
		else
		{
			LogPool::Warning(LogEvent::Socket, "http error fun", httpProtocol);
			response = BuildResponse(e.Code, e.ResponseJson);
		}
		SendTcp(socket, response, NULL);
		return ProtocolPacket(AnalysisResult::Request, 0, static_cast<unsigned int>(packetSize + length), 0, 0);
	}
}