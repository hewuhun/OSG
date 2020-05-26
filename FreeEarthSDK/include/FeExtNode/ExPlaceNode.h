/**************************************************************************************************
* @file ExPlaceNode.h
* @note 标记节点(可以带图标)的定义，用于在场景中显示标记
* @author c00005
* @data 2016-8-3
**************************************************************************************************/
#ifndef FE_EXTERN_PLACE_NODE_H
#define FE_EXTERN_PLACE_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExPointNode.h>

#include <osgEarth/GeoData>
#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthSymbology/TextSymbol>

namespace FeExtNode
{
	/**
	* @class CExPlaceNodeOption
	* @note 标记节点的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExPlaceNodeOption : public CExPointNodeOption
	{
	public:
		/**  
		  * @note Text节点的配置选项的构造函数
		*/
		CExPlaceNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExPlaceNodeOption();

	public:
		/**  
		  * @note 获取标记的图片的路径
		*/
		std::string& imagePath() { return m_strImage; }
		
		/**  
		  * @note 获取标记的图片的大小
		*/
		osg::Vec2& imageSize() { return m_vecSize; }

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
		double                      m_dTextSize;

		/// 文字颜色
		osg::Vec4d                  m_vecTextColor;
	
		///标记的图片
		std::string					m_strImage;

		///标记的图片的大小
		osg::Vec2					m_vecSize;
	};
}

namespace FeExtNode
{
	using namespace osgEarth::Annotation;

	/**
	* @class CExPlaceNode
	* @note 标记节点类，主要用于绘制标记对象
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExPlaceNode : public CExPointNode
	{
	public:
		/**  
		  * @note 点节点的构造函数
		  * @param pMapNode [in] 渲染上下文
		  * @param opt [in] 配置属性集合
		*/
		CExPlaceNode(FeUtil::CRenderContext* pRender, CExPlaceNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExPlaceNode();

	public:
		/**  
		  * @note 设置标记的内容
		  * @param strName [in] 标记的内容
		*/
		virtual void SetName(const std::string& strName);

		/**  
		  * @note 获取标记的图标的路径
		  * @return std::string 标记的路径
		*/
		virtual std::string GetImagePath();

		/**  
		  * @note 设置标记的图标的路径
		  * @param strImage [in] 标记的图标的路径
		*/
		virtual void SetImagePath(const std::string& strImage);

		/**  
		  * @note 获取标记的图标的大小
		  * @return osg::Vec2 标记图标的大小
		*/
		virtual osg::Vec2 GetImageSize();

		/**  
		  * @note 设置标记图标的大小
		  * @param osg::Vec2 [in] 标记图标的大小
		*/
		virtual void SetImageSize(const osg::Vec2& vecSize);

		/**  
		  * @note 设置标记显示的位置
		  * @param geoPosition [in] 标记显示的位置，其中可以设置绝对高度和相对高度
		*/
		virtual void SetPosition(const osgEarth::GeoPoint& geoPosition);

		/**  
		  * @note 获取标记显示的位置  
		  * @return osgEarth::GeoPoint 返回的是标记显示的位置
		*/
		virtual osgEarth::GeoPoint GetPosition();

		/**  
		  * @note 设置标记显示的风格
		  * @param geoPosition [in] 标记显示的风格，其中可以设置标记的字体、大小、颜色、编码格式等
		*/
		virtual void SetTextSymbol(TextSymbol& symbolText);
		
		/**  
		  * @note 获取标记显示的风格
		  * @return TextSymbol 返回的是标记显示的风格
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
		virtual CExPlaceNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @note 重写了PlaceNode的计算包围球的函数
		*/
		virtual osg::BoundingSphere computeBound() const;

	protected:

		///标记节点
		osg::ref_ptr<PlaceNode>					m_rpPlaceNode;
	};
}


#endif //FE_EXTERN_PLACE_NODE_H
