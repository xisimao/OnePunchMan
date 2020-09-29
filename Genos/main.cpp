#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        string arg(argv[1]);
        if (arg.compare("flow") == 0)
        {
            JsonDeserialization jd("appsettings.json");
            LogPool::Init(jd);
            SqliteLogger logger(jd.Get<string>("Flow:Db"));
            LogPool::AddLogger(&logger);
            FlowStartup startup;
            startup.Start();
            startup.Join();
        }
        else if (arg.compare("event") == 0)
        {
            JsonDeserialization jd("appsettings.json");
            LogPool::Init(jd);
            SqliteLogger logger(jd.Get<string>("Event:Db"));
            LogPool::AddLogger(&logger);
            EventStartup startup;
            startup.Start();
            startup.Join();
        }
        else if (arg.compare("io") == 0)
        {
            JsonDeserialization jd("appsettings.json");
            LogPool::Init(jd);
            IoAdapter adapter;
            adapter.Start();
            adapter.Join();
        }
    }
    return 0;
}