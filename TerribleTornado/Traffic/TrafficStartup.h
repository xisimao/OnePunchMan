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
#include "SqliteLogger.h"
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
        * ���캯��
        */
        TrafficStartup();

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

    protected:
        void StartCore();

        /**
        * ��ʼ���̼߳���
        * @param mqtt mqtt
        * @param decodes �����༯��
        * @param detectors ��ͨ����༯��
        * @param detects ��Ƶ����༯��
        * @param recogns ��Ƶʶ���༯��
        * @param loginHandler ��½���
        */
        virtual void InitThreads(MqttChannel* mqtt, std::vector<DecodeChannel*>* decodes, std::vector<TrafficDetector*>* detectors, std::vector<DetectChannel*>* detects, std::vector<RecognChannel*>* recogns,int loginHandler) = 0;

        /**
        * ��ʼ��ͨ������
        */
        virtual void InitChannels() = 0;

        /**
        * �������ݿ�
        */
        virtual void UpdateDb() = 0;

        /**
        * ��ȡͨ��json����
        * @param host �����ַ
        * @param channelIndex ͨ�����
        * @return ͨ��json����
        */
        virtual std::string GetChannelJson(const std::string& host, int channelIndex)=0;

        /**
        * ����ͨ������
        * @param e http��Ϣ�����¼�����
        */
        virtual void SetDevice(HttpReceivedEventArgs* e)=0;

        /**
        * ����ͨ��
        * @param e http��Ϣ�����¼�����
        */
        virtual void SetChannel(HttpReceivedEventArgs* e)=0;

        /**
        * ɾ��ͨ��
        * @param channelIndex ͨ�����
        * @return ɾ���ɹ�����true,���򷵻�false
        */
        virtual bool DeleteChannel(int channelIndex)=0;

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

        //�����̼߳���,������Ƶ����
        std::vector<DecodeChannel*> _decodes;

    private:
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

        //��ͨ����༯��,������Ƶ����
        std::vector<TrafficDetector*> _detectors;
        //��Ƶ����̼߳���,������Ƶ����
        std::vector<DetectChannel*> _detects;
        //��Ƶʶ���̼߳���,������Ƶ����/RecognChannel::ItemCount
        std::vector<RecognChannel*> _recogns;
    };
}

