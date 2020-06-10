#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("flow") == 0)
        {
            LogPool::Init("appsettings.json");

            FlowStartup channel;
            channel.Startup();
        }
        else if (arg.compare("event") == 0)
        {
            LogPool::Init("appsettings.json");
            EventStartup channel;
            channel.Startup();
        }
        else if (arg.compare("io") == 0)
        {
            LogPool::Init("io.json");
            IoAdapter adapter;
            adapter.Start();
            adapter.Join();
        }
    }
    return 0;
}