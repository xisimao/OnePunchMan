#include "LogReader.h"

using namespace std;
using namespace OnePunchMan;

tuple<vector<LogItem>, int> LogReader::ReadLogs(const std::string logDirectory, const string& logName, const DateTime& logDate, int logLevel, int logEvent, int pageNum, int pageSize,bool hasTotal)
{
    vector<LogItem> result;
    int count = 0;

    string filePath = Path::Combine(logDirectory,FileLogger::GetLogFileName(logName,logDate));

    ifstream file;
    file.open(filePath);
    if (file.good())
    {
        int skipLines = (pageNum - 1) * pageSize;
        string preLine;
        getline(file, preLine);

        while (true)
        {
            string line;
            getline(file, line);
            if (preLine.empty())
            {
                break;
            }
            else if (line.empty() || line[0]=='[')
            {
                if (logLevel != 0 && preLine[26] != logLevel + 48)
                {
                    preLine = line;
                    continue;
                }
        
                string logEventTag = StringEx::Combine("[", logEvent, "]");
 
                if (logEvent != 0 && preLine.find(logEventTag, 27)==string::npos)
                {
                    preLine = line;
                    continue;
                }
                count += 1;
                if (skipLines > 0)
                {
                    skipLines -= 1;
                }
                else if (pageSize > 0)
                {
                    pageSize -= 1;
                    vector<string> datas=StringEx::Split(preLine, "]");
                    //4¸ö]1¸ö¿Õ¸ñ
                    int headLength = static_cast<int>(datas[0].size() + datas[1].size() + datas[2].size() + 4);
                    LogItem item;
                    item.Time = datas[0].substr(1, datas[0].size() - 1);
                    item.Level = datas[1].substr(1, datas[1].size() - 1);
                    item.Event = datas[2].substr(1, datas[2].size() - 1);
                    item.Content = preLine.substr(headLength, preLine.size() - headLength);
                    result.push_back(item);
                }
                else
                {
                    if (!hasTotal)
                    {
                        break;
                    }
                }
                preLine = line;
            }
            else
            {
                preLine += line;
            }
        }
    }
    file.close();
    return tuple<vector<LogItem>, int>(result, hasTotal?count:0);
}
