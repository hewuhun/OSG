/**************************************************************************************************
* @file FeDraggerAttribute.h
* @note 编辑器属性类
* @author g00034
* @data 2016-10-28
**************************************************************************************************/

#ifndef FE_EXTERN_DRAGGER_ATTRI_H
#define FE_EXTERN_DRAGGER_ATTRI_H

#include <osg/LineWidth>

namespace osgGA
{
	class GUIEventAdapter;
	class GUIActionAdapter;
}

namespace FeNodeEditor
{
	/**
	  * @class CFeDraggerAttribute
	  * @brief 编辑器属性类
	  * @note 编辑器属性类，编辑器继承该类来实现统一的属性变换
	  * @author g00034
	*/
	class CFeDraggerAttribute
	{
	public:
		CFeDraggerAttribute(void);
		
	protected:
		virtual ~CFeDraggerAttribute(){}

		/**  
		  * @note 由编辑器调用，实现编辑器属性设置
		*/
		bool HandleAttributes( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

		/**  
		  * @note 设置线宽
		*/
		void setLineWidth(float linePixelWidth);

	private:
		/**  
		  * @note 创建属性对象
		*/
		void CreateAttributes();

	protected:
		/// 线宽
		float						 m_fLineWidth;

		/// 线宽属性
		osg::ref_ptr<osg::LineWidth> m_rpLineWidth;
	};

}

#endif // FE_EXTERN_DRAGGER_ATTRI_H