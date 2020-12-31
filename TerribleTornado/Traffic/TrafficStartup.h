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
#include "FlowDetector.h"
#include "EventDetector.h"
#include "SocketMaid.h"
#include "TrafficData.h"
#include "SqliteLogger.h"
#ifndef _WIN32
#include "clientsdk.h"
#endif // !_WIN32

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class TrafficStartup 
        : public IObserver<HttpReceivedEventArgs>
        , public IObserver<MqttDisconnectedEventArgs>
    {
    public:

        /**
        * ���캯��
        */
        TrafficStartup();

        /**
        * ��������
        */
        ~TrafficStartup();

        //ͨ������
        static const int ChannelCount;
        //����߳�����
        static const int DetectCount;
        //ʶ���߳�����
        static const int RecognCount;

        /**
        * http��Ϣ�����¼�����
        * @param e http��Ϣ�����¼�����
        */
        virtual void Update(HttpReceivedEventArgs* e);

        /**
        * mqtt�Ͽ��¼�����
        * @param e mqtt�Ͽ��¼�����
        */
        void Update(MqttDisconnectedEventArgs* e);

        /**
        * ����ϵͳ
        */
        void Start();
    private:
        /**
        * ��ȡͨ��json����
        * @param host �����ַ
        * @param channelIndex ͨ�����
        * @return ͨ��json����
        */
        std::string GetChannelJson(const std::string& host, int channelIndex);

        /**
        * ����ͨ������
        * @param e http��Ϣ�����¼�����
        */
        void SetDevice(HttpReceivedEventArgs* e);

        /**
        * ����ͨ��
        * @param e http��Ϣ�����¼�����
        */
        void SetChannel(HttpReceivedEventArgs* e);

        /**
        * ɾ��ͨ��
        * @param channelIndex ͨ�����
        * @return ɾ���ɹ�����true,���򷵻�false
        */
        bool DeleteChannel(int channelIndex);

        /**
        * ���ͨ��������
        * @param channel ͨ������
        * @return ���ؿ��ַ�����ʾ���ͨ��,���򷵻ش���ԭ��
        */
        std::string CheckChannel(TrafficChannel* channel);

        /**
        * �ж�ͨ������Ƿ����
        * @param channelIndex ͨ�����
        * @return ����ture��ʾͨ����ſ���,���򷵻�false
        */
        bool ChannelIndexEnable(int channelIndex);

        /**
        * ����ͨ�����Json�ַ���
        * @param channelJson Ҫ����json�ַ���
        * @param channel ͨ������
        * @param host ������ַ
        */
        void FillChannelJson(std::string* channelJson, const TrafficChannel* channel, const std::string& host);
        
        /**
        * ����ͨ��json���ͨ������
        * @param channel Ҫ����ͨ������
        * @param jd json�ַ���
        */
        void FillChannel(TrafficChannel* channel, const JsonDeserialization& jd);
        
        /**
        * ����ͨ������json���ͨ������
        * @param channel Ҫ����ͨ������
        * @param jd json�ַ���
        * @param itemIndex ͨ��������������
        */
        void FillChannel(TrafficChannel* channel, const JsonDeserialization& jd, int itemIndex);

        /**
        * ��ȡurl�Ƿ���ָ����ǰ׺
        * @param url url
        * @param key ǰ׺
        * @return ����true��ʾurl��ָ��ǰ׺
        */
        bool UrlStartWith(const std::string& url, const std::string& key);

        /**
        * ��ȡurl�е�id
        * @param url url
        * @param key ǰ׺
        * @return id
        */
        std::string GetId(const std::string& url, const std::string& key);

        /**
         * ��ʼ���̼߳���
         * @param loginHandler ��½���
         */
        void InitThreads(int loginHandler);

        /**
        * ��ʼ��ͨ������
        */
        void InitChannels();

        /**
        * ��ѯ�豸
        * @param e http��Ϣ�����¼�����
        */
        void GetDevice(HttpReceivedEventArgs* e);

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
        //�¼���������߳�
        DataChannel* _data;
        //����
        EncodeChannel* _encode;
        //�����̼߳���,������Ƶ����
        std::vector<DecodeChannel*> _decodes;
        //��������༯��,������Ƶ����
        std::vector<FlowDetector*> _flowDetectors;
        //�¼�����༯��,������Ƶ����
        std::vector<EventDetector*> _eventDetectors;
        //��Ƶ����̼߳���,������Ƶ����
        std::map<int,DetectChannel*> _detects;
        //��Ƶʶ���̼߳���,������Ƶ����/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;


    };
}

