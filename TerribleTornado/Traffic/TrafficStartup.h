#pragma once
#include "Thread.h"
#include "Observable.h"
#include "JsonFormatter.h"
#include "HttpHandler.h"
#include "Command.h"
#include "DecodeChannel.h"
#include "FlowDetector.h"
#include "EventDetector.h"
#include "SocketMaid.h"
#include "TrafficData.h"
#ifndef _WIN32
#include "clientsdk.h"
#endif // !_WIN32

namespace OnePunchMan
{
    //����ϵͳ�����߳�
    class TrafficStartup
        : public IObserver<HttpReceivedEventArgs>
    {
    public:
        /**
        * ���캯��
        */
        TrafficStartup();

        /**
        * ��������
        */
        virtual ~TrafficStartup();

        //ͨ������
        static const int ChannelCount;

        /**
        * http��Ϣ�����¼�����
        * @param e http��Ϣ�����¼�����
        */
        virtual void Update(HttpReceivedEventArgs* e);

        /**
        * ����ϵͳ
        */
        virtual void Start() = 0;

    protected:
        /**
        * ������ʵ�ֵĸ���ͨ��
        * @param channel ͨ��
        */
        virtual void UpdateChannel(const TrafficChannel& channel);

        /**
        * ������ʵ�ֵĽ�ͼ
        * @param channelIndex ͨ�����
        */
        virtual void Screenshot(int channelIndex)=0;

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

    private:
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
        * ���ͨ��
        * @param channelIndex ͨ�����
        */
        void ClearChannel(int channelIndex);

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
        bool CheckChannelIndex(int channelIndex);

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
        * ��ȡͨ��json����
        * @param channel ͨ������
        * @param host ������ַ
        */
        std::string GetChannelJson(const std::string& host,int channelIndex);

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
        * ��ѯ�豸
        * @param e http��Ϣ�����¼�����
        */
        void GetDevice(HttpReceivedEventArgs* e);
    };
}

