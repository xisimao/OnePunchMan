#include "SeemmoSDK.h"

using namespace std;
using namespace OnePunchMan;

seemmo_process_init_t SeemmoSDK::seemmo_process_init=NULL;
seemmo_uninit_t SeemmoSDK::seemmo_uninit = NULL;
seemmo_thread_init_t SeemmoSDK::seemmo_thread_init = NULL;
seemmo_thread_uninit_t SeemmoSDK::seemmo_thread_uninit = NULL;
seemmo_video_pvc_t SeemmoSDK::seemmo_video_pvc = NULL;
seemmo_video_pvc_recog_t SeemmoSDK::seemmo_video_pvc_recog = NULL;
seemmo_version_t SeemmoSDK::seemmo_version = NULL;

void* SeemmoSDK::Handle = NULL;

bool SeemmoSDK::Init()
{
#ifdef _WIN32
	return false;
#else 
	Handle = dlopen("/mtd/seemmo/programs/aisdk/3559a/lib/libexport_sdk.so", RTLD_LAZY);
	if (Handle == NULL)
	{
		LogPool::Error(LogEvent::System, "初始化seemmo sdk失败");
		return false;
	}
	seemmo_process_init = (seemmo_process_init_t)dlsym(Handle, "seemmo_process_init");
	seemmo_uninit = (seemmo_uninit_t)dlsym(Handle, "seemmo_uninit");
	seemmo_thread_init = (seemmo_thread_init_t)dlsym(Handle, "seemmo_thread_init");
	seemmo_thread_uninit = (seemmo_thread_uninit_t)dlsym(Handle, "seemmo_thread_uninit");
	seemmo_video_pvc = (seemmo_video_pvc_t)dlsym(Handle, "seemmo_video_pvc");
	seemmo_video_pvc_recog = (seemmo_video_pvc_recog_t)dlsym(Handle, "seemmo_video_pvc_recog");
	seemmo_version = (seemmo_version_t)dlsym(Handle, "seemmo_version");

	if (seemmo_process_init == NULL
		|| seemmo_uninit == NULL
		|| seemmo_thread_init == NULL
		|| seemmo_thread_uninit == NULL
		|| seemmo_video_pvc == NULL
		|| seemmo_video_pvc_recog == NULL
		|| seemmo_version == NULL
		)
	{
		LogPool::Error(LogEvent::System, "初始化seemmo sdk函数失败");
		return false;
	}
	else
	{
		int32_t result = seemmo_process_init("/mtd/seemmo/programs/aisdk", 8, 8, "", 1, false);
		if (result != 0)
		{
			LogPool::Error(LogEvent::System, "打开seemmo sdk失败，错误代码:", result);
			return false;
		}
	}
	LogPool::Information(LogEvent::System, "初始化seemmo sdk成功");
	return true;
#endif // !_WIN32


}

void SeemmoSDK::Uninit()
{
#ifndef _WIN32
	if (seemmo_uninit != NULL)
	{
		seemmo_uninit();
	}
	if (Handle != NULL)
	{
		dlclose(Handle);
	}
#endif // !_WIN32
	LogPool::Information(LogEvent::System,"卸载seemmo sdk");
}