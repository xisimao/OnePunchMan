#include "TrafficData.h"

using namespace std;
using namespace OnePunchMan;

string TrafficData::_dbName("");

TrafficData::TrafficData()
	:_sqlite(_dbName)
{

}

void TrafficData::Init(const std::string& dbName)
{
	_dbName = dbName;
}

string TrafficData::LastError()
{
	return _sqlite.LastError();
}

string TrafficData::GetParameter(const string& key)
{
	string sql(StringEx::Combine("Select Value From System_Parameter Where Key='", key, "'"));
	SqliteReader sqlite(_dbName);
	string value;
	if (sqlite.BeginQuery(sql))
	{
		if (sqlite.HasRow())
		{
			value = sqlite.GetString(0);
		}
		sqlite.EndQuery();
	}
	return value;
}
bool TrafficData::SetParameter(const string& key, const string& value)
{
	string sql(StringEx::Combine("Update System_Parameter Set Value='", value, "' Where Key='", key, "'"));
	return _sqlite.ExecuteRowCount(sql) == 1;
}

void TrafficData::UpdateDb()
{
	SqliteReader sqlite(_dbName);
	string sql(StringEx::Combine("Select * From System_Parameter"));
	if (!sqlite.BeginQuery(sql))
	{
		_sqlite.ExecuteRowCount("CREATE TABLE [System_Parameter] ([Key] TEXT NOT NULL, [Value] TEXT NOT NULL, CONSTRAINT[PK_Flow_Parameter] PRIMARY KEY([Key]))");
		_sqlite.ExecuteRowCount("Insert Into System_Parameter (Key,Value) Values ('Version','')");
		_sqlite.ExecuteRowCount("Insert Into System_Parameter (Key,Value) Values ('VersionValue','')");
		_sqlite.ExecuteRowCount("Insert Into System_Parameter (Key,Value) Values ('SN','')");
	}
}