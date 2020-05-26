/**************************************************************************************************
* @file MotionAlgorithm.h
* @note 模型运动相关算法
* @author w00024
* @data 2016-4-9
**************************************************************************************************/
#ifndef FE_MOTION_ALGORITHM_H
#define FE_MOTION_ALGORITHM_H

#include <FeAlg/Export.h>
#include <osg/Group>
#include <FeAlg/MotionStateInfo.h>

namespace FeAlg
{
	/**
	* @brief 根据节点名称，递归查找节点
	* @param node [in] 根节点指针
	* @param strName [in] 需要查找节点的名称
	* @return 查找成功返回节点指针，失败返回NULL
	*/
	extern FEALG_EXPORT osg::Node *RecursionNode(osg::Node *node, const std::string &strName);

	/**
	* @brief 根据输入的地表信息，获取其中的关键点
	* @param vecStateInfo [in] 输入的点集
	* @param vecKeyPoints [out] 输出的点集
	* @return 空
	*/
	extern FEALG_EXPORT void CalculateKeyPointsFromPath(const std::vector<FeAlg::CMotionStateInfo>& vecSateInfo, std::vector<osg::Vec3d>& vecKeyPoints, double dDiffYawDegree = 1.0, double dDiffPitchDegree = 0.2);

	/** 
	* @brief 保存点集到目标文件 
	*/
	extern FEALG_EXPORT bool WritePathToFile(const std::string& strFile, const std::vector<FeAlg::CMotionStateInfo>& outStateInfo, const std::vector<osg::Vec3d> &outEditPoints);

	/**
	* @brief 从目标文件中读取点集 
	*/
	extern FEALG_EXPORT bool ReadFileToPath(const std::string& strFile, std::vector<FeAlg::CMotionStateInfo>& inputStateInfo, std::vector<osg::Vec3d> &inputEditPoints);

	/**
	* @brief 将输入的点集在xOy平面上旋转rotateRadian。
	*/
	extern FEALG_EXPORT bool PointRotation(const std::vector<osg::Vec3d> & in, std::vector<osg::Vec3d>& out, double rotateRadian);

	/**
	* @brief 将输入的点在xOy平面上旋转rotateRadian。
	*/
	extern FEALG_EXPORT bool PointRotation(const osg::Vec3d& in, osg::Vec3d& out, double rotateRadian);
}

#endif //FE_MOTION_ALGORITHM_H
