#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
//#include "clientsdk.h"

using namespace std;
using namespace OnePunchMan;

int main1()
{
    //int r=vas_sdk_startup();
    //cout << r << endl;
    return 0;
}
int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("flow") == 0)
        {
            LogPool::Init("appsettings.json");
            FlowStartup startup;
            startup.Start();
            startup.Join();
        }
        else if (arg.compare("event") == 0)
        {
            LogPool::Init("appsettings.json");
            EventStartup startup;
            startup.Start();
            startup.Join();
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