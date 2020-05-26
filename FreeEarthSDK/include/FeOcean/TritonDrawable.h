/**************************************************************************************************
* @file TritonDrawable.h
* @note Triton 海洋绘制节点类
* @author g00034
* @data 2017-3-3
**************************************************************************************************/

#ifndef TRITON_DRAWABLE__HH__
#define TRITON_DRAWABLE__HH__

#include <osg/Drawable>
#include <osg/Texture2D>
#include <osgViewer/Viewer>
#include <osgEarth/ThreadingUtils>
#include <osgEarth/Map>
#include <FeTriton/Environment.h>
#include <OpenThreads/Thread>

namespace FeOcean
{
	class TritonContext;
	class TritonNode;

	/**
	  * @class TritonDrawable
	  * @brief 海洋节点绘制类
	  * @author g00034
	*/
	class TritonDrawable : public osg::Drawable
	{
		META_Object(osg, TritonDrawable)

	public:
		/**
		  *@note: 构造函数
		*/
		TritonDrawable(){}
		TritonDrawable(osgViewer::View * viewer,TritonNode* tn); 
		TritonDrawable(const TritonDrawable &, const osg::CopyOp &){}
		virtual ~TritonDrawable();

		/**
		  *@note: 绘制函数
		*/
		virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

		/**
		  *@note: 根据当前相机位置，设置包围球大小
		*/
		void setTritonBound(const osg::Vec3d& camPos);

	private:
		osg::BoundingBox                m_bbox;         // 包围盒
		mutable OpenThreads::Mutex      m_mutex;        // 互斥对象
		osg::observer_ptr<TritonNode>   m_opTritonNode; // 海洋节点
	
	};


	/**
	  * @class GodRayDrawable
	  * @brief GodRay绘制节点
	  * @author g00034
	*/
	class GodRayDrawable : public osg::Drawable
	{
		META_Object(osg, GodRayDrawable)

	public:
		/**
		  *@note: 构造函数
		*/
		GodRayDrawable() {}
		GodRayDrawable(const GodRayDrawable&, const osg::CopyOp &){}
		GodRayDrawable(TritonNode* tn); 
		virtual ~GodRayDrawable() {}

		/**
		  *@note: 绘制函数
		*/
		virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

	private:
		mutable OpenThreads::Mutex    m_mutex;         // 互斥对象
		osg::observer_ptr<TritonNode> m_opTritonNode;  // 海洋节点
	};


}

#endif // TRITON_DRAWABLE__HH__

