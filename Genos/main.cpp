#include "TrafficStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    SqliteLogger logger(jd.Get<string>("traffic.db"));
    LogPool::AddLogger(&logger);
    TrafficStartup startup;
    startup.Start();
    startup.Join();
    return 0;
}