/**************************************************************************************************
* @file RenderContext.h
* @note 定义了渲染系统的环境
* @author l00008
* @data 2014-11-03
**************************************************************************************************/

#ifndef SCENE_RENDER_CONTEXT_H
#define SCENE_RENDER_CONTEXT_H

#include <osgViewer/View>
#include <osgViewer/CompositeViewer>
#include <osgEarth/MapNode>

#include <FeUtils/Export.h>
#include <FeUtils/FreeViewPoint.h>

namespace FeUtil
{
	/**
	* @class CRenderContext
	* @brief 封装渲染系统的环境
	* @note 封装了三维渲染系统的最底层环境，此类仅仅供内部使用，不作为系统外部使用。
	* @author l00008
	*/

	class FEUTIL_EXPORT CRenderContext : public osg::Referenced
	{
	public:
		/**  
		* @brief 构造函数
		* @note 详细说明  
		* @param pView [in] 主视口View
		* @param pRoot [in] 根节点
		* @param pMapNode [in] 地球节点
		*/
		CRenderContext(osgViewer::CompositeViewer* pViewer, osg::Group* pRoot, osgEarth::MapNode* pMapNode);
		CRenderContext();

		virtual ~CRenderContext(void);

	public:
		/**
		*@note: 获得View，此View是主视口View
		*/
		virtual osgViewer::View* GetView(int nIndex = 0);

		/**
		*@note: 获得Viewer
		*/
		virtual osgViewer::CompositeViewer* GetViewer();
		virtual void SetViewer(osgViewer::CompositeViewer* pViewer);

		/**
		*@note: 获得主Root，这个节点是整个场景的Root节点，一般不推荐对其操作
		*/
		virtual osg::Group* GetRoot();
		virtual void SetRoot(osg::Group* pRoot);

		/**
		*@note: 获得osgEarth的mapNode
		*/
		virtual osgEarth::MapNode* GetMapNode();
		virtual void SetMapNode(osgEarth::MapNode* pMapNode);

		/**
		*@note: 获得FeTriiton
		*/
		virtual osg::Node* GetTritonNode();
		virtual void SetTritonNode(osg::Node* pNode);

		/**
		*@note:获得投影，其实是从MapNode获得
		*/
		virtual const osgEarth::SpatialReference* GetMapSRS();

		/**
		*@note: 获得以及设置场景渲染系统，在系统构建完成之后需要设定渲染系统
		*/
		virtual osg::GraphicsContext* GetGraphicsWindow(int nIndex = 0);

		/**
		*@note: 获得主相机
		*/
		osg::Camera* GetCamera(int nIndex = 0);

		typedef std::list<osg::observer_ptr<osg::Node> > OceanReflectionList;
		/**
		*@note: 获得海洋反射节点列表
		*/
		OceanReflectionList& GetOceanReflectionList()
		{
			return m_oceanReflectionList;
		}

		typedef std::list<osg::observer_ptr<osg::Node> > ManipulatorIntersectionList;
		/**
		*@note: 获得地球碰撞节点列表
		*/
		ManipulatorIntersectionList& GetManipulatorIntersectionList()
		{
			return m_manipulatorIntersectionList;
		}	

		/**
		*@note: 设置相机距离地面的高度
		*/
		void setCamAltFromTerrain(double alt)
		{
			m_camAltFromTerrain = alt;
		}
		/**
		*@note: 获取相机距离地面的高度
		*/
		double getCamAltFromTerrain()
		{
			return m_camAltFromTerrain;
		}

		/**
		*@note: 获得地球（mapNode）自转MT
		*/
		void SetEarthRotationMT(osg::MatrixTransform* pErMT);
		osg::MatrixTransform* GetEarthRotationMT();
	public:

		/**  
		* @brief 注册和移除事件处理Handler
		* @note 统一管理注册和移除处理事件的Handler
		* @return 成功true，失败false
		*/
		bool AddEventHandler(osgGA::GUIEventHandler* pHandler, int nIndex = 0);
		bool RemoveEventHandler(osgGA::GUIEventHandler* pHandler, int nIndex = 0);

	protected:
		///root节点
		osg::observer_ptr<osg::Group>					m_opRoot;    

		///地球场景节点
		osg::observer_ptr<osgEarth::MapNode>			m_opMapNode;    

		//场景视景器
		osg::observer_ptr<osgViewer::CompositeViewer>	m_opCompositeViewer;

		///场景视景器
		//osg::observer_ptr<osgViewer::View>				m_opView;    

		///相机距地面的高度
		double                                          m_camAltFromTerrain;

		///漫游器求交节点列表
		ManipulatorIntersectionList                     m_manipulatorIntersectionList; 


		///海洋节点
		osg::observer_ptr<osg::Node>                    m_opTriton;  

		///海洋反射节点列表
		OceanReflectionList                             m_oceanReflectionList; 

		///地球自转MT
		osg::observer_ptr<osg::MatrixTransform>			m_opEarthRotationMT;	
	};

	class FEUTIL_EXPORT CRenderContextObserver
	{
	public:
		CRenderContextObserver(CRenderContext* pContext):m_opRenderContext(pContext){};

		virtual ~CRenderContextObserver(){};

	public:
		virtual CRenderContext* GetRenderContext(){return m_opRenderContext.get();}

		virtual  void SetRenderContext(CRenderContext* pContext ){m_opRenderContext = pContext;}

	protected:
		osg::observer_ptr<CRenderContext>      m_opRenderContext;
	};
}

#endif//FESHELL_SYSTEM_SERVICE_H
