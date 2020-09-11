#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
//#include "clientsdk.h"

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
            TrafficDirectory::Init(jd);
            TrafficData::Init(jd.Get<string>("Flow:Db"));
            SqliteLogger logger(TrafficData::DbName);
            LogPool::AddLogger(&logger);

            FlowStartup startup;
            startup.Start();
            startup.Join();
        }
        else if (arg.compare("event") == 0)
        {
            JsonDeserialization jd("appsettings.json");
            LogPool::Init(jd);
            TrafficDirectory::Init(jd);
            TrafficData::Init(jd.Get<string>("Event:Db"));
            SqliteLogger logger(TrafficData::DbName);
            LogPool::AddLogger(&logger);
            EventStartup startup;
            startup.Start();
            startup.Join();
        }
        else if (arg.compare("io") == 0)
        {
            JsonDeserialization jd("appsettings.json");
            LogPool::Init(jd);
            TrafficDirectory::Init(jd);
            TrafficData::Init(jd.Get<string>("Flow:Db"));

            IoAdapter adapter;
            adapter.Start();
            adapter.Join();
        }
    }
    return 0;
}