#include "SeemmoSDK.h"

using namespace std;
using namespace TerribleTornado;
using namespace Saitama;

seemmo_process_init_t SeemmoSDK::seemmo_process_init=NULL;
seemmo_uninit_t SeemmoSDK::seemmo_uninit = NULL;
seemmo_thread_init_t SeemmoSDK::seemmo_thread_init = NULL;
seemmo_thread_uninit_t SeemmoSDK::seemmo_thread_uninit = NULL;
seemmo_video_pvc_t SeemmoSDK::seemmo_video_pvc = NULL;
seemmo_video_pvc_recog_t SeemmoSDK::seemmo_video_pvc_recog = NULL;

bool SeemmoSDK::Inited = false;
void* SeemmoSDK::Handle = NULL;

bool SeemmoSDK::Init()
{
#ifndef _WIN32
	Handle = dlopen("/mtd/seemmo/programs/aisdk/3559a/lib/libexport_sdk.so", RTLD_LAZY);
	if (Handle == NULL)
	{
		LogPool::Error(LogEvent::Detect, "init seemmo sdk failed");
		return false;
	}
	seemmo_process_init = (seemmo_process_init_t)dlsym(Handle, "seemmo_process_init");
	seemmo_uninit = (seemmo_uninit_t)dlsym(Handle, "seemmo_uninit");
	seemmo_thread_init = (seemmo_thread_init_t)dlsym(Handle, "seemmo_thread_init");
	seemmo_thread_uninit = (seemmo_thread_uninit_t)dlsym(Handle, "seemmo_thread_uninit");
	seemmo_video_pvc = (seemmo_video_pvc_t)dlsym(Handle, "seemmo_video_pvc");
	seemmo_video_pvc_recog = (seemmo_video_pvc_recog_t)dlsym(Handle, "seemmo_video_pvc_recog");

	if (seemmo_process_init == NULL
		|| seemmo_uninit == NULL
		|| seemmo_thread_init == NULL
		|| seemmo_thread_uninit == NULL
		|| seemmo_video_pvc == NULL
		|| seemmo_video_pvc_recog == NULL
		)
	{
		LogPool::Error(LogEvent::Detect, "init func failed");
		return false;
	}
	else
	{
		int32_t result = seemmo_process_init("/mtd/seemmo/programs/aisdk", 8, 8, "192.168.201.66:12821", 1, 0);
		if (result == 0)
		{
			Inited = true;
		}
		else
		{
			LogPool::Error(LogEvent::Detect, "init process failed", result);
			return false;
		}
	}
#endif // !_WIN32
	LogPool::Information("init seemmo sdk");
	return true;

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
	LogPool::Information("uninit seemmo sdk");
}