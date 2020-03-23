//#include "DataChannel.h"
//#include "SocketMaid.h"
//#include "HttpHandler.h"
#include <iostream>
#include <math.h>

//using namespace Saitama;
using namespace std;

int main()
{

	int x1 = 0, x2 = 0, x3 = 0, x4 = 0;
	int y1 = 0, y2 = 0, y3 = 0, y4 = 0;

	float d = static_cast<float>((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
	// If d is zero, there is no intersection
	if (d == 0) return 0;

	// Get the x and y
	int pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
	int x = static_cast<int>(round(static_cast<float>(pre * (x3 - x4) - (x1 - x2) * post) / d));
	int y = static_cast<int>(round(static_cast<float>(pre * (y3 - y4) - (y1 - y2) * post)/ d));

	// Check if the x and y coordinates are within both lines
	if (x < std::min(x1, x2) || x > std::max(x1, x2) ||
		x < std::min(x3, x4) || x > std::max(x3, x4)) return 0;
	if (y < std::min(y1, y2) || y > std::max(y1, y2) ||
		y < std::min(y3, y4) || y > std::max(y3, y4)) return 0;

    /*SocketMaid maid(2);
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
    channel.Stop();*/

    return 0;
}
