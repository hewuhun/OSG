/**************************************************************************************************
* @file LandMotionAlgorithm.h
* @note 模型运动相关算法
* @author w00024
* @data 2016-04-09
**************************************************************************************************/
#ifndef FE_LAND_MOTION_ALGORITHM_H
#define FE_LAND_MOTION_ALGORITHM_H

#include <osg/Group>
#include <osg/Vec3d>
#include <osg/Math>
#include <osg/Matrix>

#include <FeAlg/Export.h>
#include <FeAlg/MotionStateInfo.h>
#include <FeUtils/RenderContext.h>

namespace FeAlg
{	
	/**
	* @brief 路面插值，根据获取的高程进行插值
	* @note 第一步，进行拐角处插值压入点间距，进行球面插值，插值的间距是disSphereInter
			第二步，更新计算偏航角，偏航角只与经纬度有关，与高度无关，同时更新每一点的高度
			第三步，在两个经纬度之间进行插值，插值的间距是disMaxDis
	* @param pRenderContext [in] 三维系统的基础数据
	* @param vecInput [in] 未进行插值的关键点
	* @param vecOutput [out] 插值后的关键点
	* @param disLineInter [in] 直线两点之间进行球面插值的距离
	* @param disPressIn [in] 拐角处进行插值时，计算压入点的位置
	* @param disMaxDis [in] 拐角处进行插值两点之间的最短距离。
	* @return 成功失败标志
	*/
	extern FEALG_EXPORT bool LandInterpolationAndSphereCornerSmooth(FeUtil::CRenderContext *pRenderContext, const std::vector<osg::Vec3d>& vecInput,
		std::vector<CMotionStateInfo>& vecOutput, double disSphereInter = 100.0, double disPressIn = 20.0, double disMaxDis = 1.0);

	/**
	* @brief 路面插值，根据获取的高程进行插值
	* @note 第一步，进行球面插值，插值的间距是disSphereInter
			第二步，更新计算偏航角，偏航角只与经纬度有关，与高度无关，同时更新每一点的高度
			第三步，在两个经纬度之间进行插值，插值的间距是disMaxDis
	* @param pRenderContext [in] 三维系统的基础数据
	* @param vecInput [in] 未进行插值的关键点
	* @param vecOutput [out] 插值后的关键点
	* @param disLineInter [in] 直线两点之间进行球面插值的距离
	* @param disPressIn [in] 拐角处进行插值时，计算压入点的位置
	* @param disMaxDis [in] 拐角处进行插值两点之间的最短距离。
	* @return 成功失败标志
	*/
	extern FEALG_EXPORT bool LandInterpolationAndSphereSmooth(FeUtil::CRenderContext *pRenderContext, const std::vector<osg::Vec3d>& vecInput,
		std::vector<CMotionStateInfo>& vecOutput, double disSphereInter = 100.0, double disMaxDis = 5.0);

	/**
	* @brief 球面插值，利用地球半径进行粗略插值，只与经纬度有关
	* @param vecInput [in] 输入的点集
	* @param vecOutput [out] 插值后的点集
	* @param disSphereInter [in] 插值的间距
	* @return 空
	*/
	extern FEALG_EXPORT void InterpolationSphereRhumb(const std::vector<osg::Vec3d>& vecInput, std::vector<CMotionStateInfo>& vecOutput, double disSphereInter);
	extern FEALG_EXPORT void InterpolationSphereRhumb(const osg::Vec3d start, const osg::Vec3d end,
		std::vector<CMotionStateInfo>& output, double deltaDistance);
	extern FEALG_EXPORT void InterpolationSphereRhumbAndCorner(FeUtil::CRenderContext *pRenderContext, const std::vector<osg::Vec3d>& vecInput, std::vector<CMotionStateInfo>& vecOutput, double disPressIn, double disSphereInter, double disMaxDis);

	/**
	* @brief 在两点之间根据地形进行插值，插值间距是disMaxDis，输出的点集中不包含开始点与结束点
	* @param pRenderContext [in]
	* @param stateStart,stateEnd [in] 输入的两点
	* @param vecOutput [out] 输出的点集
	* @param disMaxDis [in] 插值的最小间距
	* @return 空
	*/
	extern FEALG_EXPORT void LandInterpolationBetweenTwoLLH(FeUtil::CRenderContext *pRenderContext, CMotionStateInfo stateStart,
		CMotionStateInfo stateEnd, std::vector<CMotionStateInfo>& vecOutput, double disMaxDis /*= 5.0*/);

	/**
	* @brief 计算开始点处的偏航角
	* @param pRenderContext [in] 用到其中的mapNode
	* @param first[in] 开始点
	* @param second[in] 结束点
	* @return 空
	*/
	extern FEALG_EXPORT void CalculateYawRadian(FeUtil::CRenderContext *pRenderContext, CMotionStateInfo &first, const CMotionStateInfo &second);

	/**
	* @brief 计算三维中两点的欧式距离，返回的为long double类型
	* @param first [in] 第一点
	* @param second [in] 第二点
	* @return 距离值
	*/
	extern FEALG_EXPORT long double DistanceTwoXYZ(const osg::Vec3d &first, const osg::Vec3d& second);

	/**
	* @brief 获取两点的球面距离，只与经纬度有关
	* @param first [in] 第一点的度数
	* @param second [in] 第二点的度数
	* @return 两点的距离
	*/
	extern FEALG_EXPORT long double DistanceTwoLL(const osg::Vec3d &first, const osg::Vec3d& second);
}

namespace FeAlg
{
	/**
	* @brief 根据输入的时间，获取模型位置信息
	* @param pRenderContext [in] 地表数据
	* @param vecStateInfo [inout] 模型运动路径，更改其中的时间信息
	* @param dTime [in] 当前时间
	* @param dSpeed [in] 模型运动速度
	* @param output [out] 模型的位置信息
	* @param lastPosition [inout] 记录了上次模型的关键点位置，开始查找位置
	* @return 成功失败标志
	*/
	extern FEALG_EXPORT bool FindPositionFromLandPath(FeUtil::CRenderContext *pRenderContext, std::vector<CMotionStateInfo>& vecStateInfo, double dTime,
		double dSpeed, CMotionStateInfo& output, int& lastPosition);

	/**
	* @brief 根据模型的前后左右点，计算当前位置的俯仰角与倾斜角
	* @param pRenderContext [in] 地表数据，利用其中的mapNode
	* @param curState [inout] 当前位置信息
	* @param front,back,left,right [in] 当前位置的前后左右偏移量
	* @return 成功失败标志
	*/
	extern FEALG_EXPORT bool CalculateSlopeAndPitchRadian(FeUtil::CRenderContext *pRenderContext, CMotionStateInfo& curState, osg::Vec3d front, osg::Vec3d back, osg::Vec3d left, osg::Vec3d right);

	/**
	* @brief 计算某一点的俯仰姿态
	* @param pRenderContext [in] 地表数据信息
	* @param first [in] 第一点的经纬度，度数
	* @param second [in] 第二点的经纬度，度数
	* @param pitchRadian [out] 俯仰角
	* @return 空
	*/
	extern FEALG_EXPORT void CalculatePicthRadian(FeUtil::CRenderContext *pRenderContext, osg::Vec3d first, osg::Vec3d second, double &pitchRadian);
	
}

#endif //FE_LAND_MOTION_ALGORITHM_H
