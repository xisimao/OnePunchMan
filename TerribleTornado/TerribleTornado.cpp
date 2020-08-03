#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "HisiEncodeChannel.h"
#include "OutputHandler.h"
#include "DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    LogPool::Init("appsettings.json");
    FlowStartup flow;
    flow.Start();
    system("pause");

    return 0;
}
