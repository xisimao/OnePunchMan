#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{

    LogPool::Init("appsettings.json");
    string s = DateTime::ParseTimeStamp(DateTime::UtcNowTimeStamp()).ToString();;

    EventStartup traffic;
    traffic.Start();
    system("pause");

    return 0;
}
