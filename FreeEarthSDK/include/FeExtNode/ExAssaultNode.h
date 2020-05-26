/**************************************************************************************************
* @file ExAssaultNode.h
* @note 突击方向图元的基类
* @author w00040
* @data 2016-12-1
**************************************************************************************************/
#ifndef FE_EXTERN_ASSAULT_NODE_H
#define FE_EXTERN_ASSAULT_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExStraightArrowNode.h>
#include <FeAlg/FlightMotionAlgorithm.h>

namespace FeExtNode
{
	enum TypePosition
	{
		OUT_ARROW = 0, //外箭头
		IN_ARROW, //内箭头
		TAIL_ARROW //箭尾
	};
}

namespace FeExtNode
{
	class FEEXTNODE_EXPORT CExAssaultNodeOption : public CExStraightArrowNodeOption
	{
	public:
		/**  
		  * @note 突击方向图元节点的配置选项的构造函数
		*/
		CExAssaultNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExAssaultNodeOption();
	};
}

namespace FeExtNode
{
	class FEEXTNODE_EXPORT CExAssaultNode : public CExStraightArrowNode
	{
	public:
		/**  
		  * @note 突击方向图元节点基类的构造函数
		  * @param pMapNode [in] 地球节点
		  * @param opt [in] 突击方向图元节点的配置属性集合
		*/
		CExAssaultNode(FeUtil::CRenderContext* pRender, CExAssaultNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExAssaultNode();

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
		  * @note 绘制突击方向
		*/
		void DrawAssaultDirection();

		/**
		  * @note 计算突击方向坐标
		*/
		void ComputeCoordinate(int nFlag, const osg::Vec3d startPoint, const osg::Vec3d endPoint, osg::Vec3d &leftPoint, osg::Vec3d &rightPoint);

		/**
		  * @note 绘制内部图元细节
		*/
		virtual void DrawInternalExtra() { DrawAssaultDirection(); }

	};
}

#endif