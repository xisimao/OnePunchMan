#pragma once
#include <string>

namespace OnePunchMan
{
	//�����豸
	class FlowDevice
	{
	public:
		//��Ȩ״̬
		bool LicenceStatus;
		//Ӳ��ʹ��(GB)
		std::string DiskUsed;
		//Ӳ������(GB)
		std::string DiskTotal;
		//����汾
		std::string SoftwareVersion;
		//�豸sn
		std::string SN;

	};
}


