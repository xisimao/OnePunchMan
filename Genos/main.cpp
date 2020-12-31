#include "TrafficStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    TrafficStartup startup;
    startup.Start();
    return 0;
}