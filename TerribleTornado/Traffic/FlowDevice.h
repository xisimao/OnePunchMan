#pragma once
#include <string>

namespace OnePunchMan
{
	//流量设备
	class FlowDevice
	{
	public:
		//授权状态
		bool LicenceStatus;
		//硬盘使用(GB)
		std::string DiskUsed;
		//硬盘总数(GB)
		std::string DiskTotal;
		//软件版本
		std::string SoftwareVersion;
		//设备sn
		std::string SN;

	};
}


