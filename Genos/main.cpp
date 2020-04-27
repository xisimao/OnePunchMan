#include "DataChannel.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        string arg(argv[1]);
        if (arg.compare("stop") == 0)
        {
            SocketMaid maid(2);
            SocketHandler handler;
            maid.AddConnectEndPoint(EndPoint("127.0.0.1", 7772), &handler);
            maid.Start();
            this_thread::sleep_for(chrono::seconds(3));
            stringstream ss;
            ss << "GET /api/system HTTP/1.1\r\n"
                << "\r\n"
                << "\r\n";
            maid.SendTcp(EndPoint("127.0.0.1", 7772), ss.str());
        }
    }
    else
    {
        DataChannel channel;
        channel.Start();
        channel.Join();
    }

    return 0;
}