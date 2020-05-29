#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"

using namespace std;
using namespace OnePunchMan;

int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    FlowStartup channel;
    channel.Startup();
    return 0;
}