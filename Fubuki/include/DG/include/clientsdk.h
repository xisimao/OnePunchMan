#ifndef CLIENTSDK_H
#define CLIENTSDK_H
//#include "vas_type.h"



#define FRAME_TYPE_I	0//I֡
#define FRAME_TYPE_P	1//P֡
#define FRAME_TYPE_B	2//B֡
#define FRAME_TYPE_A	3//��Ƶ֡


#define MESSAGE_TYPE_LOGIN_OUT				0
#define MESSAGE_TYPE_MEDIA_DISCONNECT		1


typedef void (RealDataCallBackFunc)(int play_fd, int frame_type, char* buf, unsigned int size, void* usr);
typedef void (MessageCallbackFunc)(int msg_type, int param1, int param2, void* user);


#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************
���� 	 ��rtsp_sdk_startup
���� 	 ��SDK��ʼ��
���� 	 ��NULL
����     ��0: �ɹ�
           other:ʧ��
***************************************************************/

int vas_sdk_startup();


/***************************************************************
���� 	 ��vas_sdk_login
���� 	 ����¼����
���� 	 ��ip:����IP��ַ
		   port: ����˿ں�
		   username: �û���
		   password: ����
����     ��>= 0: ��¼���
           < 0:ʧ��
***************************************************************/

int vas_sdk_login(char* ip, int port, char* username, char* password);

/***************************************************************
���� 	 ��vas_sdk_set_message_callback
���� 	 ��������Ϣ�ص�
���� 	 ��cb:�ص�����
		   user: �û�����
����     ����
***************************************************************/

void vas_sdk_set_message_callback(MessageCallbackFunc cb, void* user);


/***************************************************************
���� 	 ��vas_sdk_realplay
���� 	 ������ʵʱ��Ƶ
���� 	 ��login_id: ��¼���
		   channel_id: ͨ��ID
		   func: ʵʱ���ص�
		   usr: �û�����
����     ��>= 0: Ԥ�����
           < 0:ʧ��
***************************************************************/

int vas_sdk_realplay(int login_id, char* channel_id, RealDataCallBackFunc func, void* usr);



/***************************************************************
���� 	 ��vas_sdk_stop_realplay
���� 	 ��ֹͣʵʱ��Ƶ
���� 	 ��play_id: Ԥ�����
����     ��= 0: �ɹ�
           other:ʧ��
***************************************************************/

int vas_sdk_stop_realplay(int play_id);


/***************************************************************
���� 	 ��vas_sdk_reboot()
���� 	 ����������(�������ػ��������������)
���� 	 ��null
����     ��null
***************************************************************/

void vas_sdk_reboot(int login_id);


#ifdef __cplusplus
}
#endif
#endif
