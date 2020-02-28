#include "HttpHandler.h"

using namespace std;
using namespace Saitama;

SocketHandler* HttpHandler::CloneCore()
{
	HttpHandler* handler= new HttpHandler();
	handler->Requested.Copy(&this->Requested);
	return handler;
}

SocketHandler::ProtocolPacket HttpHandler::HandleCore(int socket, unsigned int ip, unsigned short port, string::const_iterator begin, string::const_iterator end)
{
	string httpProtocol(begin, end);
	vector<string> lines = StringEx::Split(httpProtocol, "\r\n",false);
	HttpEventArgs e;
	e.Socket = socket;
	if (lines.empty())
	{
		LogPool::Warning("http empty", httpProtocol);
		return ProtocolPacket(AnalysisResult::Empty, 0, static_cast<unsigned int>(httpProtocol.size()), 0,0);
	}
	else
	{
		vector<string> columns = StringEx::Split(lines[0], " ", true);
	 
		string func = StringEx::ToUpper(columns[0]);
		//跨域
		string origin;
		for (unsigned int i = 1; i < lines.size(); ++i)
		{
			vector<string> columns = StringEx::Split(lines[i], ":", true);
			if (columns.size() > 1 && StringEx::ToUpper(columns[0]).compare("ORIGIN") == 0)
			{
				origin = StringEx::Trim(lines[i].substr(7, lines[i].size() - 7));
				break;
			}
		}

		e.Url = columns[1];
		size_t start = lines[0].size() + 2;

		if (func.compare("OPTIONS") == 0)
		{
			//找到空行，确定总长度
			bool hasEmpty = false;
			for (unsigned int i=1; i < lines.size(); ++i)
			{
				start += lines[i].size() + 2;
				if (lines[i].empty())
				{
					hasEmpty = true;
					break;
				}
			}
			if (hasEmpty)
			{
				e.Code = HttpCode::OK;
				stringstream ss;
				ss << "HTTP/1.1 " << static_cast<int>(e.Code) << "\r\n"
					<< "Vary: Origin\r\n"
					<< "Vary: Access-Control-Request-Method\r\n"
					<< "Vary: Access-Control-Request-Headers\r\n"
					<< "Access-Control-Allow-Origin: " << origin << "\r\n"
					<< "Access-Control-Allow-Methods: GET,POST,DELETE,PUT\r\n"
					<< "Access-Control-Allow-Headers: content-type\r\n"
					<< "Access-Control-Allow-Credentials: true\r\n"
					<< "Access-Control-Max-Age: 3600\r\n"
					<< "Allow: GET, HEAD, POST, PUT, DELETE, OPTIONS, PATCH\r\n"
					<< "Content-Type: application/json;charset=UTF-8\r\n"
					<< "Content-Length: 0\r\n"
					<< "Date: " << DateTime::UtcNow().ToString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n"
					<< "\r\n"
					<< e.Response;
				SendTcp(socket, ss.str(), NULL);
				return ProtocolPacket(AnalysisResult::Request, 0, static_cast<unsigned int>(start), 0, 0);
			}
			else
			{
				LogPool::Warning("options not found empty", httpProtocol);
				return ProtocolPacket(AnalysisResult::Half, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
			}
		}
		else if (func.compare("POST") == 0)
		{
			//分析body位置
			unsigned int length = 0;
			unsigned int i = 1;
			//找到长度
			bool hasLength = false;
			for (; i < lines.size(); ++i)
			{
				start += lines[i].size() + 2;
				vector<string> values = StringEx::Split(lines[i], ":", true);
				if (values.size() > 1 && StringEx::ToUpper(values[0]).compare("CONTENT-LENGTH") == 0)
				{
					StringEx::Convert<unsigned int>(values[1], &length);
					//已经处理过该行，需要对序号加1，否则下面的循环的第一个还是这行
					++i;
					hasLength = true;
					break;
				}
			}
			if (hasLength)
			{
				bool hasEmpty = false;
				//根据空行定位到开始位置
				for (; i < lines.size(); ++i)
				{
					start += lines[i].size() + 2;
					if (lines[i].empty())
					{
						hasEmpty = true;
						break;
					}
				}
				if (hasEmpty)
				{
					//半包
					if (start + length > httpProtocol.size())
					{
						return ProtocolPacket(AnalysisResult::Half, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
					}
					else
					{
						e.Request = httpProtocol.substr(start, length);
						//需要从外边获取code和response
						Requested.Notice(&e);
						stringstream ss;
						ss << "HTTP/1.1 " << static_cast<int>(e.Code) << "\r\n"
							<< "Vary: Origin\r\n"
							<< "Vary: Access-Control-Request-Method\r\n"
							<< "Vary: Access-Control-Request-Headers\r\n"
							<< "Access-Control-Allow-Origin: " << origin << "\r\n"
							<< "Access-Control-Allow-Methods: GET,POST,DELETE,PUT\r\n"
							<< "Access-Control-Allow-Headers: content-type\r\n"
							<< "Access-Control-Allow-Credentials: true\r\n"
							<< "Access-Control-Max-Age: 3600\r\n"
							<< "Allow: GET, HEAD, POST, PUT, DELETE, OPTIONS, PATCH\r\n"
							<< "Content-Type: application/json;charset=UTF-8\r\n"
							<< "Content-Length: " << e.Response.size() << "\r\n"
							<< "Date: " << DateTime::UtcNow().ToString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n"
							<< "\r\n"
							<< e.Response;
						SendTcp(socket, ss.str(), NULL);
						return ProtocolPacket(AnalysisResult::Request, 0, static_cast<unsigned int>(start + length), 0, 0);
					}
				}
				else
				{
					LogPool::Warning("post not found empty", httpProtocol);
					return ProtocolPacket(AnalysisResult::Half, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
				}
			}
			else
			{
				LogPool::Warning("post not found content-length", httpProtocol);
				return ProtocolPacket(AnalysisResult::Half, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
			}
		}
		else
		{
			LogPool::Warning("http error fun", httpProtocol);
			return ProtocolPacket(AnalysisResult::Empty, 0, static_cast<unsigned int>(httpProtocol.size()), 0,0);
		}
	}
}