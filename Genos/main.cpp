#include "DataChannel.h"
#include "SocketMaid.h"
#include "HttpHandler.h"

using namespace std;
using namespace Saitama;

int main()
{
    SocketMaid maid(2);
    DataChannel channel("127.0.0.1", 1884);
    HttpHandler handler;
    handler.HttpReceived.Subscribe(&channel);
    maid.AddListenEndPoint(EndPoint(7772), &handler);
    channel.Start();
    maid.Start();

    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1));
    }

    maid.Stop();
    channel.Stop();
     
    return 0;
}
