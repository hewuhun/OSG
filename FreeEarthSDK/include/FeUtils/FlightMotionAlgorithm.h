/**************************************************************************************************
* @file FlightMotionAlgorithm.h
* @note 模型运动相关算法
* @author w00024
* @data 2016-4-9
**************************************************************************************************/
#ifndef FE_FLIGHT_MOTION_ALGORITHM_H
#define FE_FLIGHT_MOTION_ALGORITHM_H

#include <osg/Vec3d>
#include <osg/Math>

#include <FeUtils/RenderContext.h>
#include <FeUtils/MiniAnimationBuilder.h>

namespace FeUtil
{
	/**
	* @brief 球面插值及拐角平滑
	* @param pRenderContext [in] 三维系统的基础数据
	* @param vecInput [in] 未进行插值的关键点
	* @param vecOutput [out] 插值后的关键点
	* @param disLineInter [in] 直线两点之间进行球面插值的距离
	* @param disPressIn [in] 拐角处进行插值时，计算压入点的位置
	* @param disMaxSmooth [in] 拐角处进行插值两点之间的最短距离。
	* @return 空
	*/
	FEUTIL_EXPORT void FlightInterpolationAndSphereSmooth(
		FeUtil::CRenderContext *pRenderContext,
		const std::vector<CMotionStateInfo>& vecInput,
		std::vector<CMotionStateInfo>& vecOutput,
		double disLineInter = 10.0,
		double disPressIn = 200.0,
		double disMaxSmooth = 10.0,
		double dRotatingPercent = 0.3, 
		double dRollRadio = 90.0 / 60.0, 
		double dRollMaxDegree = 60.0);


	/*函数重载，输入参数格式不同*/
	FEUTIL_EXPORT void FlightInterpolationAndSphereSmooth(
		FeUtil::CRenderContext *pRenderContext, 
		const std::vector<osg::Vec3d>& vecInput,
		std::vector<CMotionStateInfo>& vecOutput, 
		double dTimeLong, // 动画时长(单位：秒)
		double disLineInter = 10.0,
		double disPressIn = 200.0,
		double disMaxSmooth = 10.0, 
		double dRotatingPercent = 0.3,
		double dRollRadio = 90.0 / 60.0, 
		double dRollMaxDegree = 60.0);

	/**
	* @brief 计算向量的长度
	*/
	FEUTIL_EXPORT long double Length3D(const osg::Vec3d &point);

	/**
	* @brief 拐角处球面平滑
	* @param pRenderContext [in] 三维系统的基础数据
	* @param start [in] 开始计算点
	* @param end [in] 结束计算点
	* @param control [in] 中间控制点
	* @param output [out] 平滑插值后的点集
	* @param disMaxInter [in] 插值后两点之间的最大距离。
	* @return 空
	*/
	FEUTIL_EXPORT void InterpolationCornerSphereSmooth(
		FeUtil::CRenderContext *pRenderContext, 
		osg::Vec3d &start, 
		osg::Vec3d end,
		osg::Vec3d control, 
		std::vector<osg::Vec3d>& output, 
		double disMaxInter = 10.0);

	/* 在上面函数的基础上，新增了翻滚角的处理 */
	FEUTIL_EXPORT  void InterpolationCornerSphereSmooth(
		FeUtil::CRenderContext *pRenderContext,
		osg::Vec3d start, 
		osg::Vec3d end,
		osg::Vec3d control, 
		std::vector<CMotionStateInfo>& output,
		double disMaxInter = 10.0,
		double dRotatingPercent = 0.3,
		double dRollRadio = 90.0 / 60.0, 
		double dRollMaxDegree = 60.0);

	/**
	* @brief 拐角处平滑处理, 初始包含压入点与压出点，并按照某一点距离压入点的距离进行排序。
	* @param mapOutput [inout] 平滑后的点集
	* @param centerPosition [in] 球面的球心坐标
	* @param radius [in] 球面的半径
	* @param maxDistance [in] 插值后两点之间的最大距离。
	* @return 空
	*/
	FEUTIL_EXPORT void ProcessCornerSmooth(
		std::map<long double, osg::Vec3d>& mapOutput, 
		osg::Vec3d centerPosition, 
		long double radius, 
		long double maxDistance);

	/**
	* @brief  获取start, end之间第一个球面插值点
	* @param start [in] 开始插入的点
	* @param end [in] 插入的结束点
	* @param output [out] 第一个球面插值点
	* @param deltaDistance [in] 插入点之间的距离。
	* @return 成功失败标志
	*/
	FEUTIL_EXPORT bool GetFirstInterpolation(
		FeUtil::CRenderContext *pRenderContext,
		const osg::Vec3d start,
		const osg::Vec3d end, 
		osg::Vec3d& output, 
		double deltaDistance);

	/**
	* @brief 两点之间进行球面插值，输出的点集不包含起始点和结束点
	* @param start [in] 开始插入的点
	* @param end [in] 插入的结束点
	* @param output [out] 插入后的点集，不包含start、end点
	* @param deltaDistance [in] 插入点之间的距离。
	* @return 空
	*/
	FEUTIL_EXPORT void InterpolationSphere(
		FeUtil::CRenderContext *pRenderContext,
		const osg::Vec3d start,
		const osg::Vec3d end,
		std::vector<CMotionStateInfo>& output, 
		double deltaDistance);

	/**
	* @brief 计算某一点的俯仰姿态
	* @param pRenderContext [in] 地表数据信息
	* @param first [in] 第一点的经纬度，度数
	* @param second [in] 第二点的经纬度，度数
	* @param pitchRadian [out] 俯仰角
	* @return 空
	*/
	FEUTIL_EXPORT void CalculatePicthRadian(
		FeUtil::CRenderContext *pRenderContext, 
		osg::Vec3d first,
		osg::Vec3d second,
		double &pitchRadian);

	/**
	* @brief 计算开始点处的偏航角
	* @param pRenderContext [in] 用到其中的mapNode
	* @param first[in] 开始点
	* @param second[in] 结束点
	* @return 空
	*/
	FEUTIL_EXPORT void CalculateYawRadian(
		FeUtil::CRenderContext *pRenderContext,
		const osg::Vec3d &first,
		const osg::Vec3d &second, 
		double &yawRadian);

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
	FEUTIL_EXPORT bool FindPositionFromFlightPath(
		CRenderContext *pRenderContext, 
		std::vector<CMotionStateInfo>& vecStateInfo, 
		double dTime,
		double dSpeed,
		CMotionStateInfo& output, 
		int& lastPosition);

	/**
	* @brief 根据模型的前后左右点，计算当前位置的俯仰角与倾斜角
	* @param pRenderContext [in] 地表数据，利用其中的mapNode
	* @param curState [inout] 当前位置信息
	* @param front,back,left,right [in] 当前位置的前后左右偏移量
	* @return 成功失败标志
	*/
	FEUTIL_EXPORT bool CalculateSlopeAndPitchRadian(
		FeUtil::CRenderContext *pRenderContext,
		CMotionStateInfo& curState,
		osg::Vec3d front, 
		osg::Vec3d back, 
		osg::Vec3d left, 
		osg::Vec3d right);

	/**
	* @brief 计算三维中两点的欧式距离，返回的为long double类型
	* @param first [in] 第一点
	* @param second [in] 第二点
	* @return 距离值
	*/
	FEUTIL_EXPORT long double DistanceTwoXYZ(const osg::Vec3d &first, const osg::Vec3d& second);

	/**
	* @brief 获取两点的球面距离，只与经纬度有关
	* @param first [in] 第一点的度数
	* @param second [in] 第二点的度数
	* @return 两点的距离
	*/
	FEUTIL_EXPORT long double DistanceTwoLL(const osg::Vec3d &first, const osg::Vec3d& second);
}


#endif //FE_FLIGHT_MOTION_ALGORITHM_H

