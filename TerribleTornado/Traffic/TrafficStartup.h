#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "HttpHandler.h"
#include "Command.h"
#include "SeemmoSDK.h"
#include "MqttChannel.h"
#include "DetectChannel.h"
#include "DecodeChannel.h"
#include "TrafficDetector.h"
#include "SocketMaid.h"
#include "TrafficData.h"
#ifndef _WIN32
#include "clientsdk.h"
#endif // !_WIN32

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class TrafficStartup 
        : public ThreadObject
        , public IObserver<HttpReceivedEventArgs>
        , public IObserver<MqttDisconnectedEventArgs>
    {
    public:

        /**
        * @brief: ���캯��
        */
        TrafficStartup();

        //ͨ������
        static const int ChannelCount;
        //����߳�����
        static const int DetectCount;
        //ʶ���߳�����
        static const int RecognCount;

        /**
        * @brief: http��Ϣ�����¼�����
        * @param: e http��Ϣ�����¼�����
        */
        virtual void Update(HttpReceivedEventArgs* e);

        /**
        * @brief: mqtt�Ͽ��¼�����
        * @param: e mqtt�Ͽ��¼�����
        */
        void Update(MqttDisconnectedEventArgs* e);

    protected:
        void StartCore();

        /**
        * @brief: ��ʼ���̼߳���
        * @param: mqtt mqtt
        * @param: decodes �����༯��
        * @param: detectors ��ͨ����༯��
        * @param: detects ��Ƶ����༯��
        * @param: recogns ��Ƶʶ���༯��
        * @param: loginHandler ��½���
        */
        virtual void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<TrafficDetector*>* detectors, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns,int loginHandler) = 0;

        /**
        * @brief: ��ʼ��ͨ������
        */
        virtual void InitChannels() = 0;

        /**
        * @brief: �������ݿ�
        */
        virtual void UpdateDb() = 0;

        /**
        * @brief: ��ȡͨ��json����
        * @param: host �����ַ
        * @param: channelIndex ͨ�����
        * @return: ͨ��json����
        */
        virtual std::string GetChannelJson(const std::string& host, int channelIndex)=0;

        /**
        * @brief: ����ͨ������
        * @param: e http��Ϣ�����¼�����
        */
        virtual void SetDevice(HttpReceivedEventArgs* e)=0;

        /**
        * @brief: ����ͨ��
        * @param: e http��Ϣ�����¼�����
        */
        virtual void SetChannel(HttpReceivedEventArgs* e)=0;

        /**
        * @brief: ɾ��ͨ��
        * @param: channelIndex ͨ�����
        * @return: ɾ���ɹ�����true�����򷵻�false
        */
        virtual bool DeleteChannel(int channelIndex)=0;

        /**
        * @brief: ��ȡͨ���ļ�ⱨ��
        * @param: channelIndex ͨ�����
        * @param: e http��Ϣ�����¼�����
        */
        virtual void GetReport(int channelIndex,HttpReceivedEventArgs* e){}

        /**
        * @brief: �ж�ͨ������Ƿ����
        * @param: channelIndex ͨ�����
        * @return: ����ture��ʾͨ����ſ��ã����򷵻�false
        */
        bool ChannelIndexEnable(int channelIndex);

        /**
        * @brief: ���ͨ��������
        * @param: channel ͨ������
        * @return: ���ؿ��ַ�����ʾ���ͨ�������򷵻ش���ԭ��
        */
        std::string CheckChannel(TrafficChannel* channel);

        /**
        * @brief: ��ȡ������Ϣjson����
        * @param: field �ֶ���
        * @param: message ������Ϣ
        * @return: json����
        */
        std::string GetErrorJson(const std::string& field, const std::string& message);
        
        /**
        * @brief: ����ͨ�����Json�ַ���
        * @param: channelJson Ҫ����json�ַ���
        * @param: channel ͨ������
        * @param: host ������ַ
        */
        void FillChannelJson(std::string* channelJson, const TrafficChannel* channel, const std::string& host);
        
        /**
        * @brief: ����ͨ��json���ͨ������
        * @param: channel Ҫ����ͨ������
        * @param: jd json�ַ���
        */
        void FillChannel(TrafficChannel* channel, const JsonDeserialization& jd);
        
        /**
        * @brief: ����ͨ������json���ͨ������
        * @param: channel Ҫ����ͨ������
        * @param: jd json�ַ���
        * @param: itemIndex ͨ��������������
        */
        void FillChannel(TrafficChannel* channel, const JsonDeserialization& jd, int itemIndex);

        /**
        * @brief: ��ȡurl�Ƿ���ָ����ǰ׺
        * @param: url url
        * @param: key ǰ׺
        * @return: ����true��ʾurl��ָ��ǰ׺
        */
        bool UrlStartWith(const std::string& url, const std::string& key);

        //�����̼߳��ϣ�������Ƶ����
        std::vector<DecodeChannel*> _decodes;

    private:
        /**
        * @brief: ��ѯ�豸
        * @param: e http��Ϣ�����¼�����
        */
        void GetDevice(HttpReceivedEventArgs* e);

        /**
        * @brief: ��ȡurl�е�id
        * @param: url url
        * @param: key ǰ׺
        * @return: id
        */
        std::string GetId(const std::string& url, const std::string& key);

        //ϵͳ����ʱ��
        DateTime _startTime;
        //�㷨�Ƿ��ʼ���ɹ�
        bool _sdkInited;
        //����汾
        std::string _softwareVersion;
        //�㷨�汾
        std::string _sdkVersion;
        //socket����
        SocketMaid* _socketMaid;
        //http��Ϣ����
        HttpHandler _handler;
        //mqtt
        MqttChannel* _mqtt;

        //��ͨ����༯�ϣ�������Ƶ����
        std::vector<TrafficDetector*> _detectors;
        //��Ƶ����̼߳��ϣ�������Ƶ����
        std::vector<DetectChannel*> _detects;
        //��Ƶʶ���̼߳��ϣ�������Ƶ����/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;
    };
}

