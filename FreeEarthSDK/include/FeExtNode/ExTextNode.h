/**************************************************************************************************
* @file ExTextNode.h
* @note 文本节点的定义，用于在场景中显示文字
* @author c00005
* @data 2016-7-14
**************************************************************************************************/
#ifndef FE_EXTERN_TEXT_NODE_H
#define FE_EXTERN_TEXT_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExPointNode.h>

#include <osgEarth/GeoData>
#include <osgEarthAnnotation/LabelNode>
#include <osgEarthSymbology/TextSymbol>

namespace FeExtNode
{
	/**
	* @class CExTextNodeOption
	* @note Text节点的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExTextNodeOption : public CExPointNodeOption
	{
	public:
		/**  
		  * @note Text节点的配置选项的构造函数
		*/
		CExTextNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExTextNodeOption();

		/**  
		  * @note 文字大小
		*/
		double& TextSize() { return m_dTextSize; }

		/**  
		  * @note 文字颜色
		*/
		osg::Vec4d& TextColor() { return m_vecTextColor; }

	protected:
		/// 文字大小
		double      m_dTextSize;

		/// 文字颜色
		osg::Vec4d  m_vecTextColor;
	};
}

namespace FeExtNode
{
	using namespace osgEarth::Annotation;

	/**
	* @class CExTextNode
	* @note 文本节点类，主要用于绘制文本对象
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExTextNode : public CExPointNode
	{
	public:
		/**  
		  * @note 点节点的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 配置属性集合
		*/
		CExTextNode(FeUtil::CRenderContext* pRender, CExTextNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExTextNode();

	public:
		/**  
		  * @note 设置文本的内容
		  * @param strName [in] 文本的内容
		*/
		virtual void SetName(const std::string& strName);

		/**  
		  * @note 设置文本显示的位置
		  * @param geoPosition [in] 文本显示的位置，其中可以设置绝对高度和相对高度
		*/
		virtual void SetPosition(const osgEarth::GeoPoint& geoPosition);

		/**  
		  * @note 获取文本显示的位置  
		  * @return osgEarth::GeoPoint 返回的是文本显示的位置
		*/
		virtual osgEarth::GeoPoint GetPosition();

		/**  
		  * @note 设置文本显示的风格
		  * @param geoPosition [in] 文本显示的风格，其中可以设置文本的字体、大小、颜色、编码格式等
		*/
		virtual void SetTextSymbol(TextSymbol& symbolText);

		/**  
		  * @note 获取文本显示的风格
		  * @return TextSymbol 返回的是文本显示的风格
		*/
		virtual TextSymbol GetTextSymbol();

		/**  
		  * @note 设置文本字体颜色
		*/
		virtual void SetTextColor(const osg::Vec4f& color);

		/**  
		  * @note 获取文本字体颜色
		*/
		virtual osg::Vec4f GetTextColor();

		/**  
		  * @note 设置文本字体大小
		*/
		virtual void SetTextSize(const double& size);

		/**  
		  * @note 获取文本字体大小
		*/
		virtual double GetTextSize();

	public:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExTextNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @note 重写了LabelNode的计算包围球的函数
		*/
		virtual osg::BoundingSphere computeBound() const;

	protected:
		///文本节点
		osg::ref_ptr<LabelNode>					m_rpLabelNode;
		
	};
}

#endif //FE_EXTERN_LOD_NODE_H
