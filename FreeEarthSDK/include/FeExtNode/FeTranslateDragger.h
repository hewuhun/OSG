/**************************************************************************************************
* @file FeTranslateDragger.h
* @note 平移编辑器
* @author g00034
* @data 2016-10-18
**************************************************************************************************/

#ifndef FE_EXTERN_TRANS_DRAGGER_H
#define FE_EXTERN_TRANS_DRAGGER_H

#include <osgManipulator/TranslatePlaneDragger>
#include <FeExtNode/FeDraggerAttribute.h>
#include <FeExtNode/FeCompositeDragger.h>


namespace FeNodeEditor
{
	/**
	  * @class CExTranslate1DDragger
	  * @brief 一维平移编辑器
	  * @note 对模型平移操作
	  * @author g00034
	*/
	class CExTranslate1DDragger : public osgManipulator::Translate1DDragger, public CFeDraggerAttribute
	{
	public:
		CExTranslate1DDragger(const osg::Vec3d& s, const osg::Vec3d& e);
		~CExTranslate1DDragger();

		/** Setup default geometry for dragger. */
		void setupDefaultGeometry();

		/**  
		  * @note 实现父类接口，添加自定义功能
		*/
		virtual bool handle(const osgManipulator::PointerInfo& pi, 
			const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	
	protected:
		osg::ref_ptr<osg::Cone> _cone;
		float					_coneHeight;
		float					_pickCylinderRadius;
	};

}

namespace FeNodeEditor
{
	/**
	  * @class CExTranslatePlaneDragger
	  * @brief 面平移编辑器
	  * @note 对模型平移操作
	  * @author g00034
	*/
	class CExTranslatePlaneDragger : public osgManipulator::Translate2DDragger, public CFeDraggerAttribute
	{
	public:
		/** Setup default geometry for dragger. */
		void setupDefaultGeometry();

		/**  
		  * @note 实现父类接口，添加自定义功能
		*/
		virtual bool handle(const osgManipulator::PointerInfo& pi, 
			const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

		/**  
		  * @note 添加关联编辑器，本操作器激活的同时将高亮所有关联编辑器
		*/
		void AddAssociateDragger(CExTranslate1DDragger* dragger);

		/**  
		  * @note 设置边框颜色
		*/
		void setLineColor(const osg::Vec4& color);

	private:
		/**  
		  * @note 是否高亮所有关联编辑器
		*/
		void ActiveAssociateDraggers(bool bActive);

	protected:
		/// 关联编辑器，激活的同时高亮所有关联编辑器
        typedef std::vector<osg::observer_ptr<CExTranslate1DDragger> > ASSOCIATE_DRAGGER_LIST;
		ASSOCIATE_DRAGGER_LIST      m_associatedDraggerList;

		/// 边框线颜色
		osg::ref_ptr<osg::Vec4Array>  m_rpColorsArray;
	};
}

namespace FeNodeEditor
{
	/**
	  * @class CFeTranslateDragger
	  * @brief 平移编辑器
	  * @note 对模型平移操作
	  * @author g00034
	*/
	class CFeTranslateDragger : public CFeCompositeDragger
	{
	public:
		CFeTranslateDragger(void);
		virtual ~CFeTranslateDragger();

		/** Setup default geometry for dragger. */
		void setupDefaultGeometry();
		
		/**  
		  * @note 显示/隐藏指定轴编辑器
		*/
		void ShowAxisX(bool bShow);
		void ShowAxisY(bool bShow);
		void ShowAxisZ(bool bShow);

	protected:
		/// 坐标轴平移操作器
		osg::ref_ptr< CExTranslate1DDragger >  _xDragger;
		osg::ref_ptr< CExTranslate1DDragger >  _yDragger;
		osg::ref_ptr< CExTranslate1DDragger >  _zDragger;

		/// 面平移操作器
		osg::ref_ptr< CExTranslatePlaneDragger> _xyDragger;
		osg::ref_ptr< CExTranslatePlaneDragger> _xzDragger;
		osg::ref_ptr< CExTranslatePlaneDragger> _yzDragger;
	};

}

#endif // FE_EXTERN_TRANS_DRAGGER_H
