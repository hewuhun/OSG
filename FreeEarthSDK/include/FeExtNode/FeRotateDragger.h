/**************************************************************************************************
* @file FeRotateDragger.h
* @note 旋转编辑器
* @author g00034
* @data 2016-10-18
**************************************************************************************************/

#ifndef FE_EXTERN_ROTATE_DRAGGER_H
#define FE_EXTERN_ROTATE_DRAGGER_H

#include <osgManipulator/RotateCylinderDragger>
#include <FeExtNode/FeDraggerAttribute.h>
#include <FeExtNode/FeCompositeDragger.h>


namespace FeNodeEditor
{
	/**
	  * @class CExRotateCylinderDragger
	  * @brief 一维旋转编辑器
	  * @note 一维旋转编辑器，对模型旋转操作
	  * @author g00034
	*/
	class CExRotateCylinderDragger : public osgManipulator::RotateCylinderDragger, public CFeDraggerAttribute
	{
	public:
		CExRotateCylinderDragger();

		/** Setup default geometry for dragger. */
		void setupDefaultGeometry();

		/**  
		  * @note 实现父类接口，添加自定义功能
		*/
		virtual bool handle(const osgManipulator::PointerInfo& pi, 
			const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	
	protected:
		float _pickCylinderHeight;
		osg::ref_ptr<osg::Geode> _geode;
		osg::ref_ptr<osg::Cylinder> _cylinder;
	};

}

namespace FeNodeEditor
{
	/**
	  * @class CFeRotateDragger
	  * @brief 旋转编辑器
	  * @note 对模型旋转操作
	  * @author g00034
	*/
	class CFeRotateDragger : public CFeCompositeDragger
	{
	public:
		CFeRotateDragger(void);
		virtual ~CFeRotateDragger();

		/** Setup default geometry for dragger. */
		void setupDefaultGeometry();

	protected:
		osg::ref_ptr<CExRotateCylinderDragger> _xDragger;
		osg::ref_ptr<CExRotateCylinderDragger> _yDragger;
		osg::ref_ptr<CExRotateCylinderDragger> _zDragger;
	};

}

#endif // FE_EXTERN_ROTATE_DRAGGER_H


