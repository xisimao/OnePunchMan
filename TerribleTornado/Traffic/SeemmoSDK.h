#pragma once
#ifndef _WIN32
#include <dlfcn.h>
#endif // !_WIN32

#include "LogPool.h"

namespace OnePunchMan
{
	//获取版本
	typedef const char* (*seemmo_version_t)(void);
	//初始化进程
	typedef int32_t(*seemmo_process_init_t)(const char* base_dir,
		uint32_t img_core_num,
		uint32_t video_core_num,
		const char* auth_server,
		int32_t auth_type,
		bool log_init_swith);
	//卸载sdk
	typedef int32_t(*seemmo_uninit_t)(void);
	//初始化线程
	typedef int32_t(*seemmo_thread_init_t)(
		int type,
		int device,
		int batch);
	//卸载线程
	typedef int32_t(*seemmo_thread_uninit_t)();
	//视频检测
	typedef int32_t(*seemmo_video_pvc_t)(
		int32_t frame_num,
		const int32_t* video_chn,
		const uint64_t* timestamp,
		const uint8_t** ppbgr24,
		const uint32_t* height,
		const uint32_t* width,
		const char** calc_params,
		char* rsp_buf,
		int32_t* buff_len,
		int32_t timeout);
	//视频识别
	typedef int32_t(*seemmo_video_pvc_recog_t)(
		int32_t image_num,
		const char** guids,
		const char* calc_param,
		char* rsp_buf,
		int32_t* buff_len,
		uint8_t** ppbgr24_buf,
		int32_t timeout);

	class SeemmoSDK
	{
	public:

		static bool Init();
		static void Uninit();
		static seemmo_process_init_t seemmo_process_init;
		static seemmo_uninit_t seemmo_uninit;
		static seemmo_thread_init_t seemmo_thread_init;
		static seemmo_thread_uninit_t seemmo_thread_uninit;
		static seemmo_video_pvc_t seemmo_video_pvc;
		static seemmo_video_pvc_recog_t seemmo_video_pvc_recog;

	private:

		static void* Handle;
	};
}


