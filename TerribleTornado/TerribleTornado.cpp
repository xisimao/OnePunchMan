#include "FlowStartup.h"
#include "EventStartup.h"
#include "IoAdapter.h"
#include "EncodeChannel.h"
#include "DecodeChannel.h"
#include "SqliteLogger.h"

using namespace std;
using namespace OnePunchMan;

//车辆的距离
class CarDistance
{
public:
	CarDistance()
		:Distance(0), Length(0)
	{

	}
	double Distance;
	int Length;
};

void AddOrderedList(list<CarDistance>* distances, int length, double distance)
{
	CarDistance carDistance;
	carDistance.Distance = distance;
	carDistance.Length = length;

	if (distances->empty())
	{
		distances->push_back(carDistance);
	}
	else
	{
		for (list<CarDistance>::iterator it = distances->begin(); it != distances->end(); ++it)
		{
			if (it->Distance > carDistance.Distance)
			{
				distances->insert(it, carDistance);
				return;
			}
		}
		distances->push_back(carDistance);
	}
}

int CalculateQueueLength(const list<CarDistance>& distances)
{
	int queueLength = 0;

	if (distances.size() > 1)
	{
		list<CarDistance>::const_iterator preCar = distances.begin();
		list<CarDistance>::const_iterator nextCar = ++distances.begin();
		while (preCar != distances.end() && nextCar != distances.end())
		{
			if (nextCar->Distance - preCar->Distance > 50)
			{
				break;
			}
			else
			{
				if (queueLength == 0)
				{
					queueLength += static_cast<int>(preCar->Length);
				}

				queueLength += static_cast<int>(nextCar->Length);
			}
			preCar = nextCar;
			++nextCar;
		}
	}
	return queueLength;
}

int main(int argc, char* argv[])
{
    JsonDeserialization jd("appsettings.json");
    LogPool::Init(jd);
    TrafficDirectory::Init(jd,"1");
    TrafficData::Init(jd.Get<string>("Flow:Db"));
	FlowStartup start;
	start.Start();
	start.Join();

    return 0;
}
