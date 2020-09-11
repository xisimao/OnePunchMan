#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"
#include "SqliteLogger.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
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

    return 0;
}
