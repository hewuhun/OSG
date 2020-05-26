/**************************************************************************************************
* @file BSplineAlgorithm.h
* @note B样条曲线算法
* @author w00040
* @data 2016-11-29
**************************************************************************************************/
#ifndef FE_BSPLINE_ALGORITHM_H
#define FE_BSPLINE_ALGORITHM_H

#include <iostream>
#include <vector>
#include <osg/Vec3d>
#include <osg/Math>

#include <FeAlg/Export.h>

namespace FeAlg
{
	class FEALG_EXPORT CBSplineAlgorithm
	{
	public:
		CBSplineAlgorithm(void);

		~CBSplineAlgorithm(void);

		/**
		* @brief 重载B样条曲线构造函数
		*/
		CBSplineAlgorithm(std::vector<osg::Vec3d>& pt);

		void Computeinterpolation(void);

		void ComputeNode(void);

		void Hartley(void);

		float ComputerLen(osg::Vec3d &p);

	public:
		std::vector<osg::Vec3d> m_vecInterpolatioNode;

		unsigned int m_k;//次数

		unsigned int m_n; //控制顶点个数

		std::vector<osg::Vec3d> m_vecControlPoint;//控制顶点

		std::vector<float> m_vecU;//节点矢量
	};
}

#endif