#ifndef CLIENTSDK_H
#define CLIENTSDK_H
//#include "vas_type.h"



#define FRAME_TYPE_I	0//I帧
#define FRAME_TYPE_P	1//P帧
#define FRAME_TYPE_B	2//B帧
#define FRAME_TYPE_A	3//音频帧


#define MESSAGE_TYPE_LOGIN_OUT				0
#define MESSAGE_TYPE_MEDIA_DISCONNECT		1


typedef void (RealDataCallBackFunc)(int play_fd, int frame_type, char* buf, unsigned int size, void* usr);
typedef void (MessageCallbackFunc)(int msg_type, int param1, int param2, void* user);


#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************
函数 	 ：rtsp_sdk_startup
描述 	 ：SDK初始化
参数 	 ：NULL
返回     ：0: 成功
           other:失败
***************************************************************/

int vas_sdk_startup();


/***************************************************************
函数 	 ：vas_sdk_login
描述 	 ：登录服务
参数 	 ：ip:服务IP地址
		   port: 服务端口号
		   username: 用户名
		   password: 密码
返回     ：>= 0: 登录句柄
           < 0:失败
***************************************************************/

int vas_sdk_login(char* ip, int port, char* username, char* password);

/***************************************************************
函数 	 ：vas_sdk_set_message_callback
描述 	 ：设置消息回调
参数 	 ：cb:回调函数
		   user: 用户数据
返回     ：无
***************************************************************/

void vas_sdk_set_message_callback(MessageCallbackFunc cb, void* user);


/***************************************************************
函数 	 ：vas_sdk_realplay
描述 	 ：请求实时视频
参数 	 ：login_id: 登录句柄
		   channel_id: 通道ID
		   func: 实时流回调
		   usr: 用户数据
返回     ：>= 0: 预览句柄
           < 0:失败
***************************************************************/

int vas_sdk_realplay(int login_id, char* channel_id, RealDataCallBackFunc func, void* usr);



/***************************************************************
函数 	 ：vas_sdk_stop_realplay
描述 	 ：停止实时视频
参数 	 ：play_id: 预览句柄
返回     ：= 0: 成功
           other:失败
***************************************************************/

int vas_sdk_stop_realplay(int play_id);


/***************************************************************
函数 	 ：vas_sdk_reboot()
描述 	 ：重启服务(必须是守护进程启动情况下)
参数 	 ：null
返回     ：null
***************************************************************/

void vas_sdk_reboot(int login_id);


#ifdef __cplusplus
}
#endif
#endif
