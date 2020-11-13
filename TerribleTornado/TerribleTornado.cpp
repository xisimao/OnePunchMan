﻿#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"
#include "SqliteLogger.h"
//#include<sys/time.h>

using namespace std;
using namespace OnePunchMan;


int main(int argc, char* argv[])
{
    DateTime d1 = DateTime::ParseTimeStamp(0);
    string s = d1.ToString();
    DateTime d2=d1.ToUtcTime();
    long long t = DateTime::NowTimeStamp();
    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    TrafficDirectory::Init(jd.Get<string>("Flow:Web"));
    TrafficData::Init(jd.Get<string>("Flow:Db"));
    //SqliteLogger logger(TrafficData::DbName);
    //LogPool::AddLogger(&logger);
    FlowStartup startup;
    startup.Start();
    startup.Join();

    return 0;
}
