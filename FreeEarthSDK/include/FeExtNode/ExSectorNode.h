/**************************************************************************************************
* @file ExSectorNode.h
* @note 扇形图元的绘制类
* @author g00034
* @data 2016-8-26
**************************************************************************************************/

#ifndef FE_EXTERN_SECTOR_NODE_H
#define FE_EXTERN_SECTOR_NODE_H

#include "ExCircleNode.h"

namespace FeExtNode
{
	/**
	  * @class CExSectorNodeOption
	  * @note 扇形节点的配置选项
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExSectorNodeOption : public CExCircleNodeOption
	{
	public:
		/**  
		  * @note 扇形构造函数
		*/
		CExSectorNodeOption();

		/**  
		  * @note 扇形析构函数
		*/
		~CExSectorNodeOption();

	public:
		/**  
		  * @note 获取扇形开始角度
		*/
		double& ArcStart() {return m_dArcStart;}

		/**  
		  * @note 获取扇形结束角度
		*/
		double& ArcEnd() {return m_dArcEnd;}



	protected:
		/// 扇形开始角度
		double     m_dArcStart;

		/// 扇形结束角度
		double     m_dArcEnd;
	};

}

namespace FeExtNode
{
	/**
	  * @class CExSectorNode
	  * @note 扇形节点类，用于绘制扇形
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExSectorNode : public CExCircleNode
	{
	public:
		/**  
		  * @note 扇形节点的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 扇形节点的配置属性集合
		*/
		CExSectorNode(FeUtil::CRenderContext* pRender, CExSectorNodeOption* opt);

		/**  
		  * @note 析构函数
		*/
		~CExSectorNode(void);

		/**  
		  * @note 设置扇形开始角度
		*/
		void SetArcStart(const double& angle);      

		/**  
		  * @note 获取扇形开始角度
		*/
		double GetArcStart();

		/**  
		  * @note 设置扇形结束角度
		*/
		void SetArcEnd(const double& dAngle);   

		/**  
		  * @note 获取扇形结束角度
		*/
		double GetArcEnd();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExSectorNodeOption* GetOption();

	};

}



namespace FeExtNode
{
	/**
	  * @class CExArcNodeOption
	  * @note 弧形节点的配置选项
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExArcNodeOption : public CExSectorNodeOption
	{
	public:
		CExArcNodeOption(){}
		~CExArcNodeOption(){}
	};
}

namespace FeExtNode
{
	/**
	  * @class CExArcNode
	  * @note 弧形节点类，用于绘制弧形
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExArcNode : public CExSectorNode
	{
	public:
		/**  
		  * @note 弧形节点的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 弧形节点的配置属性集合
		*/
		CExArcNode(FeUtil::CRenderContext* pRender, CExArcNodeOption* opt);

		/**  
		  * @note 析构函数
		*/
		~CExArcNode(void);

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExArcNodeOption* GetOption();
	};

}


#endif // FE_EXTERN_SECTOR_NODE_H

