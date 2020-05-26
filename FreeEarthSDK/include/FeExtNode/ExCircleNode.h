/**************************************************************************************************
* @file ExCircleNode.h
* @note 圆形图元的绘制类
* @author g00034
* @data 2016-8-26
**************************************************************************************************/

#ifndef FE_EXTERN_CIRCLE_NODE_H
#define FE_EXTERN_CIRCLE_NODE_H

#include "ExLocalizedNode.h"

namespace FeExtNode
{
	/**
	  * @class CExCircleNodeOption
	  * @note 圆形节点的配置选项
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExCircleNodeOption : public CExLocalizedNodeOption
	{
	public:
		CExCircleNodeOption();
		~CExCircleNodeOption();

	public:
		/**  
		  * @note 获取圆的半径
		*/
		double& Radius() { return m_dRadius; }

	protected:
		/// 圆半径
		double    m_dRadius;
	};

}


namespace osgEarth{
	namespace Annotation{ class CircleNode; }
}

namespace FeExtNode
{
	/**
	  * @class CExCircleNode
	  * @note 圆形节点类，用于绘制圆形
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExCircleNode : public CExLocalizedNode
	{
	public:
		/**  
		  * @note 圆形节点的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 圆形节点的配置属性集合
		*/
		CExCircleNode(FeUtil::CRenderContext* pRender, CExCircleNodeOption* opt);

		/**  
		  * @note 析构函数
		*/
		~CExCircleNode(void);

		/**  
		  * @note 设置圆半径
		*/
		void SetRadius(const double& radius);      

		/**  
		  * @note 获取圆半径
		*/
		double GetRadius();

		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExCircleNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);


	protected:
		/**  
		  * @note 获取当前的圆节点
		*/
		osgEarth::Annotation::CircleNode* GetCircleNode();

	private:
		/**  
		  * @note 获取具体的矢量图形节点对象，不同的子类实现不同的对象
		*/
		virtual osgEarth::Annotation::LocalizedNode* GetConcreteNode();

		/**  
		  * @note 获取矢量图形的初始样式，子类可实现该方法返回不同的样式
		*/
		virtual osgEarth::Style GetConcreteNodeStyle();
	};
}


#endif // FE_EXTERN_CIRCLE_NODE_H

