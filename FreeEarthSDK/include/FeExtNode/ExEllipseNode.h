/**************************************************************************************************
* @file ExEllipseNode.h
* @note 椭圆图元的绘制类
* @author g00034
* @data 2016-8-26
**************************************************************************************************/

#ifndef FE_EXTERN_ELLIPSE_NODE_H
#define FE_EXTERN_ELLIPSE_NODE_H

#include "ExLocalizedNode.h"

namespace FeExtNode
{
	/**
	  * @class CExEllipseNodeOption
	  * @note 椭圆形节点的配置选项
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExEllipseNodeOption : public CExLocalizedNodeOption
	{
	public:
		CExEllipseNodeOption();
		~CExEllipseNodeOption();

	public:
		/**  
		  * @note 获取椭圆长半径
		*/
		double& LongRadiu() { return m_dLongRadiu; }

		/**  
		  * @note 获取椭圆短半径
		*/
		double& ShortRadiu() { return m_dShortRadiu; }

		/**  
		  * @note 获取椭圆旋转角度
		*/
		double& Ratate() { return m_dRotate; }


	protected:
		/// 椭圆长半径
		double    m_dLongRadiu;

		/// 椭圆短半径
		double    m_dShortRadiu;

		/// 椭圆旋转角度
		double    m_dRotate;
	};
}

namespace osgEarth{
	namespace Annotation{ class EllipseNode; }
}

namespace FeExtNode
{
	/**
	  * @class CExEllipseNode
	  * @note 椭圆形节点类，用于绘制椭圆形
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExEllipseNode : public CExLocalizedNode
	{
	public:
		/**  
		  * @note 椭圆形节点的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 椭圆形节点的配置属性集合
		*/
		CExEllipseNode(FeUtil::CRenderContext* pRender, CExEllipseNodeOption* opt);

		/**  
		  * @note 析构函数
		*/
		~CExEllipseNode(void);

		/**  
		  * @note 设置/获取椭圆长半径
		*/
		void SetLongRadius(const double& radius);      
		double GetLongRadius();

		/**  
		  * @note 设置/获取椭圆短半径
		*/
		void SetShortRadius(const double& radius);       
		double GetShortRadius();

		/**  
		  * @note 设置/获取椭圆旋转角度
		*/
		void SetRotate(const double& angle);
		double GetRotate(); 

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExEllipseNodeOption* GetOption();

	protected:
		/**  
		  * @note 获取当前的椭圆节点
		*/
		osgEarth::Annotation::EllipseNode* GetEllipseNode();

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

#endif // FE_EXTERN_ELLIPSE_NODE_H
