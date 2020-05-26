/**************************************************************************************************
* @file FeScaleDragger.h
* @note 缩放编辑器
* @author g00034
* @data 2016-10-18
**************************************************************************************************/

#ifndef FE_EXTERN_SCALE_DRAGGER_H
#define FE_EXTERN_SCALE_DRAGGER_H

#include <osgManipulator/Scale1DDragger>
#include <FeExtNode/FeDraggerAttribute.h>
#include <FeExtNode/FeCompositeDragger.h>

namespace FeNodeEditor
{
	/**
	  * @class CExScale1DDragger
	  * @brief 一维缩放编辑器
	  * @note 一维缩放编辑器，对模型缩放操作
	  * @author g00034
	*/
	class CExScale1DDragger : public osgManipulator::Scale1DDragger, public CFeDraggerAttribute
	{
	public:
		CExScale1DDragger();

		/** Setup default geometry for dragger. */
		void setupDefaultGeometry();

		/**  
		  * @note 实现父类接口，添加自定义功能
		*/
		virtual bool handle(const osgManipulator::PointerInfo& pi, 
			const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	};

}

namespace FeNodeEditor
{
	/**
	  * @class CScaleUniformDragger
	  * @brief 等比例缩放编辑器
	  * @note 对模型进行等比例缩放操作
	  * @author g00034
	*/
	class CScaleUniformDragger : public osgManipulator::Scale1DDragger, public CFeDraggerAttribute
	{
	public:
		CScaleUniformDragger(void);
		virtual ~CScaleUniformDragger();

		/**  
		  * @note 实现父类接口，处理拖拽事件  
		*/
		virtual bool handle(const osgManipulator::PointerInfo& pointer, 
			const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		/**  
		  * @note 使用默认的编辑器图形  
		*/
		void setupDefaultGeometry();

	private:
		/**  
		  * @note 处理内部拖拽事件  
		*/
		bool HandleInternalEvent(const osgManipulator::PointerInfo& pointer, 
			const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	};
}


namespace FeNodeEditor
{
	/**
	  * @class CFeScaleDragger
	  * @brief 缩放编辑器
	  * @note 对节点进行缩放操作
	  * @author g00034
	*/
	class CFeScaleDragger : public CFeCompositeDragger
	{
	public:
		CFeScaleDragger();
		virtual ~CFeScaleDragger();

		/**  
		  * @note 使用默认的编辑器图形  
		*/
		void setupDefaultGeometry();
	
	protected:
		// 坐标轴缩放编辑器
		osg::ref_ptr< CExScale1DDragger >        m_rpXDragger;
		osg::ref_ptr< CExScale1DDragger >        m_rpYDragger;
		osg::ref_ptr< CExScale1DDragger >        m_rpZDragger;

		// 等比缩放编辑器
		osg::ref_ptr<CScaleUniformDragger>       m_rpScaleUniform;
	};

}

#endif // FE_EXTERN_SCALE_DRAGGER_H