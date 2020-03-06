#include "Sqlite.h"

using namespace std;
using namespace Saitama;

SqliteReader::SqliteReader()
	:_stmt(NULL)
{
	sqlite3_open_v2("traffic.db", &_db, SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE,NULL);
}

SqliteReader::~SqliteReader()
{
	sqlite3_close(_db);
}

bool SqliteReader::BeginQuery(const string& sql)
{
	int result=sqlite3_prepare_v2(_db, sql.c_str(), -1, &_stmt, NULL);
	if (result == SQLITE_OK)
	{
		return true;
	}
	else
	{
		LogPool::Error(LogEvent::Sqlite, sqlite3_errmsg(_db));
		return false;
	}
}

bool SqliteReader::HasRow()
{
	if (_stmt == NULL)
	{
		return false;
	}
	else
	{
		return sqlite3_step(_stmt) == SQLITE_ROW;
	}
}


string SqliteReader::GetString(int index) const
{
	if (_stmt == NULL)
	{
		return string();

	}
	else
	{
		return reinterpret_cast<const char*>(sqlite3_column_text(_stmt, index)) == NULL ?
			string() : string(reinterpret_cast<const char*>(sqlite3_column_text(_stmt, index)));
	}
}

int SqliteReader::GetInteger(int index) const
{
	if (_stmt == NULL)
	{
		return -1;
	}
	else
	{
		return sqlite3_column_int(_stmt, index);
	}
}

void SqliteReader::EndQuery()
{
	if (_stmt != NULL)
	{
		sqlite3_finalize(_stmt);
		_stmt = NULL;
	}
}

SqliteWriter::SqliteWriter()
{
	sqlite3_open_v2("traffic.db", &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
}

SqliteWriter::~SqliteWriter()
{
	sqlite3_close(_db);
}

int SqliteWriter::ExecuteRowCount(const std::string& sql)
{
	int result = sqlite3_exec(_db, sql.c_str(),NULL,NULL,NULL);
	if (result == SQLITE_OK)
	{
		return sqlite3_changes(_db);
	}
	else
	{
		LogPool::Error(LogEvent::Sqlite,sqlite3_errmsg(_db));
		return -1;
	} 
}

int SqliteWriter::ExecuteKey(const std::string& sql)
{
	int result = sqlite3_exec(_db, sql.c_str(), NULL, NULL, NULL);
	if (result == SQLITE_OK)
	{
		return static_cast<int>(sqlite3_last_insert_rowid(_db));
	}
	else
	{
		LogPool::Error(LogEvent::Sqlite, sqlite3_errmsg(_db));
		return -1;
	}
}