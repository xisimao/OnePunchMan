#include "DG_FrameHandler.h"

using namespace std;
using namespace OnePunchMan;
#ifndef _WIN32
using namespace vega::hisi;
#endif // !_WIN32

bool DG_FrameHandler::Inited = false;

DG_FrameHandler::DG_FrameHandler(int channelIndex, int width, int height, FlowDetector* detector)
	:FrameHandler(channelIndex, width, height), _streamId(0), _detector(detector)
{
#ifndef _WIN32
	if (Inited)
	{
		DgError result = DG_Stream_Open(&_streamId, Recogn, Free, DG_OUTPUT_FAST);
		if (result == DG_OK)
		{
			LogPool::Information(LogEvent::Detect, "open stream", static_cast<int>(_streamId), static_cast<int>(result));
			DG_Function_Set_S funcion;
			funcion.enable_face = false;
			funcion.enable_vehicle = true;
			funcion.enable_pedestrian = false;
			funcion.enable_nonmotor = false;
			funcion.enable_bicycle = false;
			funcion.enable_head_shoulder = false;
			funcion.require_align2 = false;
			funcion.require_live = false;
			funcion.require_feature = false;
			funcion.require_face_attrib = false;
			funcion.require_ped_attrib = false;
			funcion.require_nonmotor_attrib = false;
			funcion.require_special_vehicle = false;
			funcion.require_plate_recog = true;
			funcion.require_car_recog = true;
			funcion.require_ped_reid = false;
			funcion.require_car_reid = false;
			funcion.require_nonmotor_reid = false;
			DG_Set_Function(_streamId, funcion);
		}
		else
		{
			_streamId = 0;
			LogPool::Error(LogEvent::Detect, "open stream failed", static_cast<int>(_streamId), static_cast<int>(result));
		}
	}

#endif // !_WIN32
}

bool DG_FrameHandler::Init()
{
#ifndef _WIN32
	//≥ı ºªØsdk
	string licence("d4c22353-56c9-4434-a724-54637cd7bf85");
	string sdkPath("/root/dgnnie_sdk/models");
	DgError result = DG_SDK_Init(licence, sdkPath, FRAME_NORMAL_MODE);
	if (result == DG_OK)
	{
		Inited = true;
		LogPool::Information(LogEvent::Detect, "init dg sdk");
		return true;
	}
	else
	{
		Inited = false;
		LogPool::Error(LogEvent::Detect, "init dg sdk failed,result:", static_cast<int>(result));
		return false;
	}
#else
	return false;
#endif // !_WIN32
}

void DG_FrameHandler::UpdateChannel(const FlowChannel& channel)
{
	_writeBmp = true;
	if (!Inited || _streamId == 0)
	{
		return;
	}
	vector<Polygon> polygons;
	for (vector<FlowLane>::const_iterator it = channel.Lanes.begin(); it != channel.Lanes.end(); ++it)
	{
		Polygon region = Polygon::FromJson(it->Region);
		if (!region.Empty())
		{
			polygons.push_back(region);
		}
	}
	if (channel.GlobalDetect)
	{
		polygons.clear();
	}
#ifndef _WIN32
	DG_Set_ROI(_streamId, DG_POLYGON_S());
	for (unsigned int i = 0; i < polygons.size(); ++i)
	{
		DG_POLYGON_S polygon;
		for (unsigned int j = 0; j < polygons[i].Points().size(); ++j)
		{
			DG_POINT_S point;
			point.s32X = polygons[i].Points()[j].X;
			point.s32Y = polygons[i].Points()[j].Y;
			polygon.astPoints.push_back(point);
		}
		DG_Set_ROI(_streamId, polygon);
	}
#endif // !_WIN32
}

#ifndef _WIN32
bool DG_FrameHandler::HandleFrame(VIDEO_FRAME_INFO_S* frame)
{
	FrameHandler::HandleFrame(frame);
	if (!Inited|| _streamId == 0)
	{
		return false;
	}
	else
	{
		if (frame == NULL)
		{
			return true;
		}
		else
		{
			int frameIndex = static_cast<int>(frame->stVFrame.u64PTS & 0xFFFFFFFF);
			if (frameIndex % 5 == 0&& _frames.size()<3)
			{
				DG_FRAME_INPUT_S dgFrame;
				dgFrame.frame_id = frameIndex;
				dgFrame.input_image = *frame;
				dgFrame.timestamp = DateTime::NowTimeStamp();
				dgFrame.user_data = this;
				DgError result = DG_Frame_Process(_streamId, Detect, dgFrame);
				if (result == DG_OK)
				{
					lock_guard<mutex> lck(_frameMutex);
					_frames.insert(pair<int, VIDEO_FRAME_INFO_S>(frameIndex, *frame));
					LogPool::Debug(LogEvent::Detect, "Process", _channelIndex, static_cast<int>(_streamId), frameIndex, _frames.size());
					return true;
				}
				else
				{
					LogPool::Warning(LogEvent::Detect, "DG_Frame_Process", static_cast<int>(_streamId), static_cast<int>(result));
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	/*if (!Inited|| _streamId == 0)
	{
		return false;
	}
	else
	{
		if (frame != NULL)
		{
			DG_FRAME_INPUT_S dgFrame;
			int frameIndex = static_cast<int>(frame->stVFrame.u64PTS & 0xFFFFFFFF);
			dgFrame.frame_id = frameIndex;
			dgFrame.input_image = *frame;
			dgFrame.timestamp = DateTime::NowTimeStamp();
			dgFrame.user_data = this;
			DgError result = DG_Frame_Process(_streamId, Detect, dgFrame);
			if (result == DG_OK)
			{
				lock_guard<mutex> lck(_frameMutex);
				_frames.insert(pair<int, VIDEO_FRAME_INFO_S>(frameIndex, *frame));
				LogPool::Debug(LogEvent::Detect, "Process",_channelIndex, static_cast<int>(_streamId), frameIndex, _frames.size());
			}
			else
			{
				LogPool::Warning(LogEvent::Detect, "DG_Frame_Process", static_cast<int>(_streamId), static_cast<int>(result));
				return false;
			}
		}
		return true;
	}*/
}

void DG_FrameHandler::Detect(DgError error, DG_STREAM_ID stream_id, const DG_FRAME_RESULT_S& frame_result)
{
	int frameIndex = static_cast<int>(frame_result.frame_input.frame_id);
	unsigned char taskId = frame_result.frame_input.input_image.stVFrame.u64PTS >> 32 & 0xFF;
	unsigned char frameSpan = frame_result.frame_input.input_image.stVFrame.u64PTS >> 40 & 0xFF;
	DG_FrameHandler* detect = static_cast<DG_FrameHandler*>(frame_result.frame_input.user_data);
	map<string, DetectItem> items;
	for (unsigned int i = 0; i < frame_result.track_retults.size(); ++i)
	{
		DetectItem item;
		if (frame_result.track_retults[i].box_.is(vega::DetectType::DETECT_TYPE_VEHICLE))
		{
			item.Type = DetectType::Car;
		}
		else if (frame_result.track_retults[i].box_.is(vega::DetectType::DETECT_TYPE_BYCICLE))
		{
			item.Type = DetectType::Bike;
		}
		else if (frame_result.track_retults[i].box_.is(vega::DetectType::DETECT_TYPE_NONMOTOR))
		{
			item.Type = DetectType::Tricycle;
		}
		else if (frame_result.track_retults[i].box_.is(vega::DetectType::DETECT_TYPE_PEDESTRIAN))
		{
			item.Type = DetectType::Pedestrain;
		}
		else
		{
			continue;
		}
		item.Region = Rectangle(frame_result.track_retults[i].box_.rect_.x, frame_result.track_retults[i].box_.rect_.y, frame_result.track_retults[i].box_.rect_.width, frame_result.track_retults[i].box_.rect_.height);
		items.insert(pair<string, DetectItem>(StringEx::ToString(frame_result.track_retults[i].track_id_), item));
	}
	detect->_detector->HandleDetect(&items, static_cast<long long>(frame_result.frame_input.timestamp), stream_id, taskId, detect->_iveBuffer, frameIndex, frameSpan);
}

void DG_FrameHandler::Recogn(DG_STREAM_ID stream_id, const DG_CAPTURE_RESULT_S& capture_result)
{
	for (unsigned int i = 0; i < capture_result.detect_objects.size(); ++i)
	{
		DG_FrameHandler* detect = static_cast<DG_FrameHandler*>(capture_result.detect_objects[i].frame_input.user_data);
		unsigned char taskId = capture_result.detect_objects[i].frame_input.input_image.stVFrame.u64PTS >> 32 & 0xFF;
		RecognItem item;
		item.Guid = StringEx::ToString(capture_result.detect_objects[i].obj_id);
		item.TaskId = taskId;
		VideoStruct_Vehicle vehicle;
		if (capture_result.detect_objects[i].recog_info.contained_objs.size() > 0
			&& capture_result.detect_objects[i].recog_info.contained_objs[0].attribute.size() >= 3)
		{
			vehicle.PlateNumber = capture_result.detect_objects[i].recog_info.contained_objs[0].attribute[2].attr_value;
		}
		for (unsigned int j = 0; j < capture_result.detect_objects[i].recog_info.attribute.size(); ++j)
		{
			int id = capture_result.detect_objects[i].recog_info.attribute[j].attr_id;
			int value = capture_result.detect_objects[i].recog_info.attribute[j].attr_matrix_value;
			if (id == 34)
			{
				vehicle.CarType = value;
			}
			else if (id == 39)
			{
				vehicle.CarColor = value;
			}
		}
		//LogPool::Information(LogEvent::Detect, vehicle.PlateNumber, vehicle.CarType, vehicle.CarColor);
		if (!vehicle.PlateNumber.empty())
		{
			detect->_detector->HandleRecognVehicle(item, NULL, vehicle);
		}
	}
}

void DG_FrameHandler::Free(DG_STREAM_ID stream_id, const std::vector<DG_FRAME_INPUT_S>& free_frames)
{
	for (unsigned int i = 0; i < free_frames.size(); ++i)
	{
		DG_FrameHandler* detect = static_cast<DG_FrameHandler*>(free_frames[i].user_data);
		lock_guard<mutex> lck(detect->_frameMutex);
		int frameIndex = static_cast<int>(free_frames[i].frame_id);
		map<int, VIDEO_FRAME_INFO_S >::iterator it = detect->_frames.find(frameIndex);
		if (it != detect->_frames.end())
		{
			//LogPool::Debug(LogEvent::Detect, "free", detect->_channelIndex, static_cast<int>(stream_id), frameIndex);
			int hi_s32_ret = HI_MPI_VPSS_ReleaseChnFrame(detect->_channelIndex - 1, 0, &it->second);
			if (hi_s32_ret != HI_SUCCESS)
			{
				LogPool::Error(LogEvent::Detect, "release", detect->_channelIndex, static_cast<int>(stream_id), frameIndex, StringEx::ToHex(hi_s32_ret));
			}
			detect->_frames.erase(it);
		}
		else
		{
			LogPool::Warning(LogEvent::Detect, "not found free frame", stream_id, frameIndex);
		}
	}
}
#endif // !_WIN32
