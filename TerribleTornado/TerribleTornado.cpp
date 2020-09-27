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
    string s("2020-09-27%2000%3A00%3A00");
    s = StringEx::Replace(s, "%20", " ");
    s = StringEx::Replace(s, "%3A", ":");

    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    TrafficDirectory::Init(jd, jd.Get<string>("Directory:FlowWeb"));
    TrafficData::Init(jd.Get<string>("Flow:Db"));
    //SqliteLogger logger(TrafficData::DbName);
    //LogPool::AddLogger(&logger);
    FlowStartup startup;
    startup.Start();
    startup.Join();

    return 0;
}
