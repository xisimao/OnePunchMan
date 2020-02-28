#include "WebSocketHandler.h"

using namespace std;
using namespace Saitama;

SocketHandler* WebSocketHandler::CloneCore()
{
	WebSocketHandler* handler = new WebSocketHandler();
	handler->Opened.Copy(&this->Opened);
	handler->Closed.Copy(&this->Closed);
	return handler;
}

SocketHandler::ProtocolPacket WebSocketHandler::HandleCore(int socket, unsigned int ip, unsigned short port, string::const_iterator begin, string::const_iterator end)
{
	string httpProtocol(begin, end);
	if (httpProtocol[0] == 'G')
	{
		WebSocketOpenedEventArgs e;
		e.Socket = socket;
		vector<string> lines = StringEx::Split(httpProtocol, "\r\n", true);
		if (lines.size())
		{
			vector<string> columns = StringEx::Split(lines[0], " ", true);
			e.Url = columns[1];
		}
		Opened.Notice(&e);
		for (unsigned int i = 1; i < lines.size(); ++i)
		{
			vector<string> columns = StringEx::Split(lines[i], ":", true);
			if (columns.size() > 1 && columns[0].compare("Sec-WebSocket-Key") == 0)
			{
				string key = StringEx::Trim(columns[1]).append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
				string backBuffer = WebSocket::ConnectBack(key, e.Code);
				SendTcp(socket, backBuffer, NULL);
			}
		}
		return ProtocolPacket(AnalysisResult::Request, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
	}
	else if (httpProtocol[0] & 0x08)
	{
		ClosedEventArgs e;
		e.Socket = socket;
		Closed.Notice(&e);
		return ProtocolPacket(AnalysisResult::Request, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
	}
	else
	{
		return ProtocolPacket(AnalysisResult::Empty, 0, static_cast<unsigned int>(httpProtocol.size()), 0, 0);
	}
}