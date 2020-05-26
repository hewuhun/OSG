/**************************************************************************************************
* @file ExRectNode.h
* @note 矩形图元的绘制类
* @author g00034
* @data 2016-8-26
**************************************************************************************************/

#ifndef FE_EXTERN_RECT_NODE_H
#define FE_EXTERN_RECT_NODE_H

#include "ExLocalizedNode.h"

namespace FeExtNode
{
	/**
	  * @class CExRectNodeOption
	  * @note 矩形节点的配置选项
	  * @author g00034
	*/
    class FEEXTNODE_EXPORT CExRectNodeOption : public CExLocalizedNodeOption
	{
	public:
		/**  
		  * @note 矩形构造函数
		*/
		CExRectNodeOption();

		/**  
		  * @note 矩形析构函数
		*/
		~CExRectNodeOption();

	public:
		/**  
		  * @note 获取矩形的高度
		*/
		double& Height() { return m_dHeight; }

			/**  
		  * @note 获取矩形的宽度
		*/
		double& Width() { return m_dWidth; }

	protected:
		/// 矩形高度
		double    m_dHeight;

		/// 矩形宽度
		double    m_dWidth;
	};

}


namespace osgEarth{
	namespace Annotation{ class RectangleNode; }
}

namespace FeExtNode
{
	/**
	  * @class CExRectNode
	  * @note 矩形节点类，用于绘制矩形
	  * @author g00034
	*/
    class FEEXTNODE_EXPORT CExRectNode : public CExLocalizedNode
	{
	public:
		/**  
		  * @note 矩形节点的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 矩形节点的配置属性集合
		*/
		CExRectNode(FeUtil::CRenderContext* pRender, CExRectNodeOption* opt);

		/**  
		  * @note 析构函数
		*/
		~CExRectNode(void);

		/**  
		  * @note 设置矩形宽度
		*/
		void SetWidth(const double& dWidth);      

		/**  
		  * @note 获取矩形宽度
		*/
		double GetWidth();

		/**  
		  * @note 设置矩形高度
		*/
		void SetHeight(const double& dHeight);      

		/**  
		  * @note 获取矩形高度
		*/
		double GetHeight();

		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExRectNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);
		
		/**
		  * @note 创建编辑工具
		*/
		//virtual FeNodeEditor::CFeNodeEditTool* CreateEditorTool();


	protected:
		/**  
		  * @note 获取当前的矩形节点
		*/
		osgEarth::Annotation::RectangleNode* GetRectNode();

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


#endif // FE_EXTERN_RECT_NODE_H

