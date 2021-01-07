#include "DG_TrafficStartup.h"

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    DG_TrafficStartup startup;
    startup.Start();
    return 0;
}

