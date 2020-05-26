/**************************************************************************************************
* @file ExAttackNode.h
* @note 进攻方向图元的基类
* @author w00040
* @data 2016-12-2
**************************************************************************************************/
#ifndef FE_EXTERN_ATTACK_NODE_H
#define FE_EXTERN_ATTACK_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExStraightArrowNode.h>
#include <FeAlg/FlightMotionAlgorithm.h>
#include <FeAlg/BSplineAlgorithm.h>

#include <iostream>
#include <vector>

namespace FeExtNode
{
	class FEEXTNODE_EXPORT CExAttackNodeOption : public CExStraightArrowNodeOption
	{
	public:
		/**  
		  * @note 进攻方向图元节点的配置选项的构造函数
		*/
		CExAttackNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExAttackNodeOption();
	};
}

namespace FeExtNode
{
	class FEEXTNODE_EXPORT CExAttackNode : public CExStraightArrowNode
	{
	public:
		/**  
		  * @note 进攻方向图元节点基类的构造函数
		  * @param pMapNode [in] 地球节点
		  * @param opt [in] 进攻方向图元节点的配置属性集合
		*/
		CExAttackNode(FeUtil::CRenderContext* pRender, CExAttackNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExAttackNode();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);
	
	public:
		/**  
		  * @note 替换线、面图元指定的末尾处的顶点   
		*/
		virtual void ReplaceBackVertex(const osg::Vec3d& vecCoord);

		/**  
		  * @note 向线、面图元后面中压入顶点
		*/
		virtual void PushBackVertex(const osg::Vec3d& vecCoord);

		/**  
		  * @note 更新编辑器
		*/
		virtual void UpdateEditor();

	private:
		/**  
		  * @note 获取矢量图形的基本构建对象，不同的子类实现不同的对象
		*/
		//virtual osgEarth::Features::Feature* GetFeature();

		/**  
		  * @note 获取矢量图形的初始样式，子类可实现该方法返回不同的样式
		*/
		//virtual osgEarth::Style GetFeatureStyle();

		/**
		  * @note 绘制进攻方向
		*/
		void DrawAttackDirection();

		/**
		  * @note 计算箭身坐标
		*/
		void ComputeArrowBody(const std::vector<osg::Vec3d> &BSplinePoints);

		/**
		  * @note 计算箭头坐标
		*/
		void ComputeArrowHead(const std::vector<osg::Vec3d> &BSplinePoints);
		
		/**
		  * @note 计算箭尾坐标
		*/
		void ComputeArrowTail(const std::vector<osg::Vec3d> &BSplinePoints);

		/**
		  * @note 动态计算线的总长度
		*/
		void ComputeSumLegth();

		/**
		  * @note 绘制内部图元细节
		*/
		virtual void DrawInternalExtra() { DrawAttackDirection(); }

	private:
		std::vector<osg::Vec3d> m_vecControl; //BSpline控制点

		double m_dDistance; //BSpline曲线的间距

		double m_dSumLength; //线段总长度
	};
}

#endif