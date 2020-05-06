#include "Sqlite.h"

using namespace std;
using namespace OnePunchMan;

SqliteReader::SqliteReader(const string& filePath)
	:_stmt(NULL)
{
	if (sqlite3_open_v2(filePath.c_str(), &_db, SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL) != 0)
	{
		LogError();
	}
}

SqliteReader::~SqliteReader()
{
	sqlite3_close(_db);
}

const string& SqliteReader::LastError()
{
	return _lastError;
}

void SqliteReader::LogError()
{
	_lastError = string(sqlite3_errmsg(_db));
	LogPool::Error(LogEvent::Sqlite, _lastError);
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
		LogError();
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

int SqliteReader::GetInt(int index) const
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

long long SqliteReader::GetLong(int index) const
{
	if (_stmt == NULL)
	{
		return -1;
	}
	else
	{
		return sqlite3_column_int64(_stmt, index);
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

SqliteWriter::SqliteWriter(const string& filePath)
{
	if (sqlite3_open_v2(filePath.c_str(), &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL) != 0)
	{
		LogError();
	}
}

SqliteWriter::~SqliteWriter()
{
	sqlite3_close(_db);
}

const string& SqliteWriter::LastError()
{
	return _lastError;
}

void SqliteWriter::LogError()
{
	_lastError = string(sqlite3_errmsg(_db));
	LogPool::Error(LogEvent::Sqlite, _lastError);
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
		LogError();
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
		LogError();
		return -1;
	}
}