//
// Created by hxu on 2018/11/19.
//

#ifndef DG_ALG_DRV_DG_SDK_DRIVER_H
#define DG_ALG_DRV_DG_SDK_DRIVER_H


#include "error.h"
#include "dg_types.h"
#include "hi_comm_video.h"

#include "opencv2/core/types.hpp"
#include <vector>
#include <string>
#include <map>
#pragma GCC visibility push(default)

namespace vega {
    namespace hisi {

        const static DG_U32 FACE_FEATURE_CNT = 384;

        const static DG_S32 DG_UNDEFINED_ATTR_ID = -1;
        const static DG_S32 DG_INVAILD_FEATURE_GROUP_ID = -1;

        typedef FrameId DG_FRAME_ID;
        typedef StreamId DG_STREAM_ID;
        typedef DG_S32 FeatureGroupId;

        typedef struct dgDG_FRAME_INPUT_S {
            DG_FRAME_ID frame_id;
            VIDEO_FRAME_INFO_S input_image;
            DG_U64 timestamp;
            void *user_data;
        } DG_FRAME_INPUT_S;

        typedef struct dgDG_FRAME_RESULT_S {
            DG_FRAME_INPUT_S frame_input;
            std::vector<TrackObject> track_retults; //track infos
            std::vector<DG_F32>      quality;
            std::vector<BBox>        head_shoulder;
        } DG_FRAME_RESULT_S;
#if 0
        typedef struct dgDG_CAPTURE_OBJ_S {
            TrackId                      obj_id;
            DG_FRAME_INPUT_S             frame_input;
            TrackObject                  detect_obj;      //tracking info
            DG_F32                       quality;
            FaceQualityFactors           face_quality_factors;
            std::vector<Feature>         feature;
            std::map<std::string,DG_F32> face_attrib_;
            std::vector<PlateInfo>       plate_result;
            VehicleBrand_Ret             brand_result;
            PedPhoneSmoke                ped_smoke_result;//only for gas station version
            std::vector<float>           ped_reid;        //only for hi3559
            std::pair<int, float>        petro_uniform;   //only for gas station version
			std::vector<float>           car_front_reid;  //only for hi3559
			std::vector<float>           car_back_reid;   //only for hi3559
        } DG_CAPTURE_OBJ_S;
#endif
        typedef struct {
            DG_S32       attr_id;
            std::string  attr_value;             //1400 standard
            DG_S32       attr_matrix_value = -1; //adapt to matrix standard
            std::string  attr_name;
            DG_F32       attr_conf;
        }DG_ATTR_INFO_S;

        typedef struct inside_obj{
            BBox                          box;
            std::vector<DG_ATTR_INFO_S>   attribute;
        }DG_INSIDE_OBJ;

        typedef struct {
            std::vector<DG_F32>           feature;        // for face right now
            std::vector<DG_F32>           reid;
            std::vector<DG_ATTR_INFO_S>   attribute;
            std::vector<DG_INSIDE_OBJ>    contained_objs; // such as plate
        } DG_RECOG_INFO_S;

        typedef struct dgDG_CAPTURE_OBJ_S {
            TrackId                      obj_id;
            DG_FRAME_INPUT_S             frame_input;
            TrackObject                  detect_obj;      //tracking info
            DG_F32                       quality;

            DG_RECOG_INFO_S              recog_info;
        } DG_CAPTURE_OBJ_S;

        typedef struct dgDG_CAPTURE_RESULT_S {
            std::vector<DG_CAPTURE_OBJ_S> detect_objects;
        } DG_CAPTURE_RESULT_S;

        typedef enum {
            DG_OUTPUT_FAST = 0,    //output right now
            DG_OUTPUT_FINE,        //output obj with best qualities
            DG_OUTPUT_DUPLICATED,  //入库模式
            DG_OUTPUT_FINE_MERGE,  //output obj with best qualities for each frame
            DG_OUTPUT_FAST_MERGE,  //output obj for each frame
            DG_STRATEGY_MAX,
        } DG_Output_Trategy_E;

        typedef struct dgDG_Function_S {
            bool enable_face;
            bool enable_vehicle;
            bool enable_pedestrian;
            bool enable_nonmotor;
            bool enable_bicycle;
            bool enable_head_shoulder;
            bool require_align2;
            bool require_live;
            bool require_feature;      //valid only if alg model exist
            bool require_face_attrib;  //valid only if alg model exist
            bool require_ped_attrib;   //valid only if alg model exist
            bool require_nonmotor_attrib;  //valid only if alg model exist
            bool require_special_vehicle; // valid only if alg model exist
            bool require_plate_recog;  //valid only if alg model exist
            bool require_car_recog;    //valid only if alg model exist
            bool require_ped_reid;     //valid only for hi3559/19
            bool require_car_reid;     //valid only for hi3559/19, must set require_car_recog TRUE to detect car pose(front/back)
            bool require_nonmotor_reid; //valid only for hi3559/19
        } DG_Function_Set_S;

        typedef struct dgDG_PlateFilter_S{
            PlateFilterSetFunc    filterfunc;
            PetroStationConf      filterconf;
        }DG_PlateFilter_Set_S;

        //ROI坐标结构体
        typedef struct
        {
            int            s32X;
            int            s32Y;
        } DG_POINT_S;

        //热区多边形结构体
        typedef struct
        {
            //int               s32Num;         //有效点数
            //TD_POINT_S        astPoints[20];   //多边形结点数组，最多支持20个点
            std::vector<DG_POINT_S> astPoints;
        } DG_POLYGON_S;

        //帧模式
        typedef enum
        {
            FRAME_NORMAL_MODE = 0,
            FRAME_CORRIDOR_MODE = 1,
        } DG_Frame_Mode_E;

        enum class DG_Model_Switcher
        {
            FACE_FEATURE = 0,
        };
        using DG_Model_Switch_Map = std::map<DG_Model_Switcher, std::string>;

        /*
         * 检测跟踪每帧结果回调函数
         */
        typedef void (*DG_FRAME_CB)(DgError error, DG_STREAM_ID stream_id, const DG_FRAME_RESULT_S &frame_result);

        /*
         * 检测跟踪抓拍结果回调函数
         */
        typedef void (*DG_CAPTURE_CB)(DG_STREAM_ID stream_id, const DG_CAPTURE_RESULT_S &capture_result);

        /*
         * 释放帧图片回调函数
         */
        typedef void (*DG_FREE_FRAMES_CB)(DG_STREAM_ID stream_id, const std::vector<DG_FRAME_INPUT_S> &free_frames);

        /*
         * 获取SDK版本号
         * @return VERSION string
         */
        std::string DG_SDK_Version();

        /*
         * SDK系统级初始化
         * @param activation_code 算法激活密钥
         * @param model_path      算法模型文件夹路径
         * @param frame_mode      画幅模式，默认正常16x9, 走廊模式为9x16
         * @return DG_OK or Err
         */
        DgError DG_SDK_Init(std::string &activation_code, const std::string &model_path, DG_Frame_Mode_E frame_mode = FRAME_NORMAL_MODE);

        /*
         * SDK切换算法模型(当同一算法提供多个模型版本时)，请在初始化之前调用
         * @param switcher        需要切换的算法类型
         * @param model_name      需要使用的算法名称(模型文件名，无需路径)
         * @return DG_OK or Err
         */
        DgError DG_SDK_Model_Select(DG_Model_Switcher switcher, const std::string &model_name);

        /*
         * SDK系统级反初始化
         * @return DG_OK or Err
         */
        DgError DG_SDK_Deinit();

        /*
         * 打开流，将检测跟踪的结果回调处理和流相关联，并关联释放回调处理
         * @param stream_id 创建返回的流ID，用以后续送帧输入
         * @param result_cb 抓拍回调函数
         * @param free_cb 释放回调函数
         */
        DgError DG_Stream_Open(DG_STREAM_ID *stream_id, DG_CAPTURE_CB result_cb, DG_FREE_FRAMES_CB free_cb, DG_Output_Trategy_E mode);

        /*
         * 检测跟踪处理函数
         * @param stream_id
         * @param frame_cb 单帧检测结果回调
         * @param input_data 用户输入数据
         */
        DgError DG_Frame_Process(DG_STREAM_ID stream_id, DG_FRAME_CB frame_cb, DG_FRAME_INPUT_S &input_data);

        /*
         * 关闭某流的检测跟踪的结果回调，释放和stream相关的资源
         * @param stream_id 待关闭的流的ID
         */
        DgError DG_Stream_Close(DG_STREAM_ID stream_id);

        /*
         * 设置最小检测框尺寸
         * @param stream_id 待设置的流
         * @type 设置对象的类型
         * @minBoxSize 设置的最小检测框尺寸
         */
        void DG_Set_MinBoxSize(DG_STREAM_ID stream_id, DetectType type, const cv::Size &minBoxSize);

        /*
         * 设置最大检测框尺寸
         * @param stream_id 待设置的流
         * @type 设置对象的类型
         * @minBoxSize 设置的最大检测框尺寸
         */
        void DG_Set_MaxBoxSize(DG_STREAM_ID stream_id, DetectType type, const cv::Size &maxBoxSize);

        /*
         * 设置送帧帧率
         * @param stream_id 待设置的流
         * @param frameRate 帧率
         */
        void DG_Set_FrameRate(DG_STREAM_ID stream_id, DG_U8 frameRate);

        /*
         * 设置画面静止对象跟踪时长
         * @param stream_id 待设置的流
         * @param trackSec 跟踪时长(秒)
         */
        void DG_Set_TrackSec(DG_STREAM_ID stream_id, DG_U32 trackSec);

        /*
         * 设置优选模式下单个对象最多输出次数（建议保持默认）
         * @param stream_id      待设置的流
         * @param bestQualityCnt 择优输出模式下，同一对象最多输出的数量（设置范围：1-3， 默认1）
         */
        void DG_Set_BestQualityCnt(DG_STREAM_ID stream_id, DG_U8 bestQualityCnt);

        /*
         * 设置优选模式下最少缓存帧数
         * @param stream_id 待设置的流
         * @param frameCnt  优选输出模式下，缓存多少帧且没有更高质量后，输出质量最优对象，默认为10
         *                  由于缓存占用MMZ内存，请根据具体内存情况设置。
         */
        void DG_Set_OutputTimeout(DG_STREAM_ID stream_id, DG_U32 frameCnt);

        /*
         * 设置优选模式下最多缓存帧数
         * @param stream_id 待设置的流
         * @param maxFrameBuf  优选输出模式下，SDK总帧数缓存(范围：>=15 默认20)
         */
        void DG_Set_MaxFrameBuf(DG_STREAM_ID stream_id, DG_U32 maxFrameBuf);

        /*
         * 设置抓拍功能开关
         * @param stream_id 待设置的流
         * @param funcion 设置的功能选项，见DG_Function_Set_S
         */
        void DG_Set_Function(DG_STREAM_ID stream_id, DG_Function_Set_S funcion);

        /*
         * 设置抓拍质量过滤
         * @param stream_id 待设置的流
         * @param type 设置的对象类型
         * @param qualityThres 设置的质量阈值
         */
        void DG_Set_QualityThres(DG_STREAM_ID stream_id, DetectType type, DG_F32 qualityThres);

        /*
         * 设置分析质量过滤，低于此阈值不做进一步分析(识别、属性、特征等)
         * 如果设置为小于抓拍质量过滤(DG_Set_QualityThres)，则只要抓拍到(通过质量过滤)就会做分析；如果设置为1，则抓拍后不做分析
         * @param stream_id 待设置的流
         * @param type 设置的对象类型
         * @param qualityThres 设置的质量阈值
         */
        void DG_Set_AnlyQualityThres(DG_STREAM_ID stream_id, DetectType type, DG_F32 qualityThres);

        /*
         * 设置人脸角度过滤
         * @param stream_id 待设置的流
         * @param pitch roll yaw 三个角度设置项，输入45即表示[-45,45]角度有效，超出此范围则丢弃
         */
        void DG_Set_FaceAnglesThres(DG_STREAM_ID stream_id, DG_F32 pitch, DG_F32 roll, DG_F32 yaw);

        /*
         * 设置人脸模糊过滤
         * @param stream_id 待设置的流
         * @param blurThres 设置的人脸模糊度阈值,范围(0-1)，值越大，人脸越清晰;
         */
        void DG_Set_BlurThres(DG_STREAM_ID stream_id, DG_F32 blurThres);
        /*
         * 设置ROI
         * @param stream_id 待设置的流
         * @param roi ROI区域
         */
        void DG_Set_ROI(DG_STREAM_ID stream_id, DG_POLYGON_S roi);

        /*
         * 设置ROI阈值
         * @param stream_id 待设置的流
         * @param roi_thres roi多边形与对象检测框重叠区域占检测框的半分比不小于该值时，认为在ROI内
         */
        void DG_Set_RoiThres(DG_STREAM_ID stream_id, DG_F32 roi_thres);

        /*
         * 设置置信度阈值(建议保持默认)
         * @param stream_id 待设置的流
         * @param type 设置的对象类型
         * @param confThres 设置的置信度阈值
         */
        void DG_Set_ConfThres(DG_STREAM_ID stream_id, DetectType type, DG_F32 confThres);

        /*
         * 特征库初始化
         * @param model_path 模型文件目录路径
         */
        void DG_FeatureMgr_Init(const std::string &model_path);

        /*
         * 创建人脸特征库组
         * @param handle 待创建特征库ID
         * @param max_feature_cnt 特征库最大特征数
         */
        void DG_FeatureMgr_CreateFaceGroup(FeatureGroupId &handle, DG_U32 max_feature_cnt);

        /*
         * 获取人脸特征库组中人脸特征数目
         * @param handle 特征库ID
         */
        DG_U32 DG_FeatureMgr_GetFaceGroupCount(FeatureGroupId handle);

        /*
         * 插入特征到特征库组
         * @param handle 特征库ID
         * @param feature 待插入人脸特征指针
         * @param face_id 待插入人脸特征标识符
         */
        DgError DG_FeatureMgr_InsertFaceGroup(FeatureGroupId handle, const std::vector<Feature> &feature, const std::string &face_id);

        /*
         * 从人脸特征库组删除特征
         * @param handle 特征库ID
         * @param face_id 待删除的人脸特征标识符
         */
        DgError DG_FeatureMgr_RemoveFeature(FeatureGroupId handle, const std::string &face_id);

        /*
         * 删除人脸特征库组
         * @param handle 待删除的的特征库ID
         */
        DgError DG_FeatureMgr_ReleaseFaceGroup(FeatureGroupId handle);

        /*
         * 从人脸特征库组中比对识别指定人脸特征
         * @param handle 特征库ID
         * @param target 待识别的人脸特征指针
         * @param match_faces 输出比对结果，string输出人脸标识符、float输出匹配分数
         * @param match_cnt 设置比对输出个数，默认1
         */
        void DG_FeatureMgr_Identify(FeatureGroupId handle, const std::vector<Feature> &target, std::unordered_map<std::string, float> &match_faces, DG_U8 match_cnt = 1);

        /*
         * 比较两个特征匹配度
         * @param feature1 人脸特征1
         * @param feature2 人脸特征2
         * @return 匹配得分，(0-99)
         */
        float DG_FeatureMgr_Compare(const std::vector<Feature> &feature1, const std::vector<Feature> &feature2);

#if !ACCESS_GRAPH

        /*
         * 打开/关闭车牌关闭处理
         * @param stream_id 待设置的流
         * @param switcher true:打开；false:关闭
         */
        void DG_Switch_PlateFilter(DG_STREAM_ID stream_id, bool switcher);
#endif
    }
}

#pragma GCC visibility pop

#endif //DG_ALG_DRV_DG_SDK_DRIVER_H
