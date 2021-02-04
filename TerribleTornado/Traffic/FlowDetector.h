#pragma once
#include <list>

#include "ImageConvert.h"
#include "TrafficData.h"
#include "Command.h"
#include "DataChannel.h"
#include "SocketMaid.h"
#include "ImageDrawing.h"


namespace OnePunchMan
{
	//ͨ�����
	class FlowDetector 
	{
	public:
		/**
		* ���캯��
		* @param width ͼƬ���
		* @param height ͼƬ�߶�
		* @param maid socket
		* @param data �����߳�
		*/
		FlowDetector(int width, int height,SocketMaid* maid,DataChannel* data);

		/**
		* ��ʼ��������������
		* @param jd json����
		*/
		static void Init(const JsonDeserialization& jd);

		/**
		* ����ͨ��
		* @param taskId ������
		* @param channel ͨ��
		*/
		void UpdateChannel(const unsigned char taskId,const TrafficChannel& channel);

		/**
		* ���ͨ��
		*/
		void ClearChannel();

		/**
		* ��ȡ�����Ƿ��ʼ���ɹ�
		* @return �����Ƿ��ʼ���ɹ�
		*/
		bool LanesInited() const;

		/**
		* ����ʱ�䷶Χ
		*/
		void ResetTimeRange();

		/**
		* ��ȡ����json����
		* @param json ���ڴ��json���ݵ��ַ���
		*/
		void GetReportJson(std::string* json);

		/**
		* ��ȡio���ݼ���
		* @param laneId ������ţ�Ϊ��ʱ��ѯ����
		* @return io���ݼ���
		*/
		std::vector<IoData> GetIoDatas(const std::string& laneId);

		/**
		* ��ȡIO״̬
		* @param laneId ������ţ�Ϊ��ʱ��ѯ����
		* @return io״̬����
		*/
		std::vector<IoData> GetIoStatus(const std::string& laneId);

		/**
		* �������
		* @param items ����������
		* @param timeStamp ʱ���
		* @param taskId ������
		* @param frameIndex ֡���
		* @param frameSpan ֡���ʱ��(����)
		*/
		void HandleDetect(std::map<std::string, DetectItem>* detectItems, long long timeStamp,unsigned char taskId, unsigned int frameIndex,unsigned char frameSpan);
		
		/**
		 * �������
		 * @param taskId ������
		 */
		void FinishDetect(unsigned char taskId);

		/**
		* ���������ʶ������
		* @param recognItem ʶ��������
		* @param iveBuffer ͼƬ�ֽ���
		* @param vehicle ������ʶ������
		*/
		void HandleRecognVehicle(const RecognItem& recognItem, const unsigned char* iveBuffer, VehicleData* vehicle);

		/**
		* ����ǻ�����ʶ������
		* @param recognItem ʶ��������
		* @param iveBuffer ͼƬ�ֽ���
		* @param bike �ǻ�����ʶ������
		*/
		void HandleRecognBike(const RecognItem& recognItem, const unsigned char* iveBuffer, BikeData* bike);

		/**
		* ��������ʶ������
		* @param recognItem ʶ��������
		* @param iveBuffer ͼƬ�ֽ���
		* @param pedestrain ����ʶ������
		*/
		void HandleRecognPedestrain(const RecognItem& recognItem, const unsigned char* iveBuffer, PedestrainData* pedestrain);
		
		/**
		* ���Ƽ������
		* @param detectItems ������
		* @param frameIndex ֡���
		*/
		void DrawDetect(const std::map<std::string, DetectItem>& detectItems, unsigned int frameIndex);

	private:
		/**
		* �����������
		* @param laneCache ��������
		* @return ������������
		*/
		FlowData CalculateMinuteFlow(FlowData* laneCache);

		/**
		* ��ӳ������������б�
		* @param distances ������������
		* @param distance ��������ֹͣ�߳���(px)
		*/
		void AddOrderedList(std::list<double>* distances,double distance);

		/**
		* �����Ŷӳ���
		* @param laneCache ������������
		* @param distances ������������
		* @return �����Ŷӳ���(m)
		*/
		int CalculateQueueLength(const FlowData& laneCache,const std::list<double>& distances);

		//IO mqtt����
		static const std::string IOTopic;
		//����mqtt����
		static const std::string FlowTopic;
		//��Ƶ�ṹ��mqtt����
		static const std::string VideoStructTopic;
		//�ϱ������ʱ��(����)
		static const int ReportMaxSpan;
		//�����Ƴ���ʱ����(����)
		static const int DeleteSpan;
		//���������Ƿ����Ŷ�״̬����С����(px)
		static int QueueMinDistance;

		//��Ƶ���
		int _channelIndex;
		//��Ƶ��ַ
		std::string _channelUrl;
		//��Ƶ���
		int _width;
		//��Ƶ�߶�
		int _height;
		//������ʼ���Ƿ�ɹ�
		bool _lanesInited;

		//socket
		SocketMaid* _maid;
		//���ݴ����߳�
		DataChannel* _data;

		//������
		int _taskId;

		//��һ֡��ʱ���
		long long _lastFrameTimeStamp;
		//��ǰ���ӵ�ʱ���
		long long _currentMinuteTimeStamp;
		//��һ���ӵ�ʱ���
		long long _nextMinuteTimeStamp;

		//��⳵������ͬ����
		std::mutex _laneMutex;
		//��⳵������
		std::vector<FlowData> _laneCaches;
		//�ϱ�����
		std::vector<FlowData> _reportCaches;
		std::vector<VehicleData> _vehicleReportCaches;
		std::vector<BikeData> _bikeReportCaches;
		std::vector<PedestrainData> _pedestrainReportCaches;

		//io���ݻ���
		std::map<std::string,std::list<IoData>> _ioDatas;

		//���ͼ��ת��
		ImageConvert _detectImage;
		//ʶ��ͼ��ת��
		ImageConvert _recognImage;

		//�Ƿ������ⱨ��
		bool _outputReport;
		//��ǰ����ķ���
		int _currentReportMinute;
		//�Ƿ����ʶ����
		bool _outputRecogn;
	};

}

