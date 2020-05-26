#ifndef FE_SCENE_MARK_HANDLER_H
#define FE_SCENE_MARK_HANDLER_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExComposeNode.h>

#include <FeUtils/EventListener.h>
#include <FeUtils/RenderContext.h>

#include <osgUtil/IntersectionVisitor>

namespace FeExtNode
{
	class CSceneMarkIntersectorVisitor : public osgUtil::IntersectionVisitor
	{
	public:
		CSceneMarkIntersectorVisitor(osgUtil::Intersector* intersector = NULL);

		~CSceneMarkIntersectorVisitor();

	public:
		//virtual void apply(osg::Billboard& geode);

		//virtual void apply(osg::LOD& lod);

		//virtual void apply(osg::PagedLOD& lod);

		//virtual void apply(osg::Projection& projection);

		virtual void apply(osg::Group& group);
	};
}


namespace FeExtNode
{
	class CFeEventListenMgr;

	/**
	  * @class CSceneMarkHandler
	  * @note 场景中图元的拾取事件处理器
	  * @author c00005
	*/
	class FEEXTNODE_EXPORT CSceneMarkHandler: 
		public CFeEventHandler
	{
	public:
		/**
        *@note: 构造函数
        */
		CSceneMarkHandler(CExComposeNode* pMarkRoot, FeUtil::CRenderContext* pRender);

		/**
        *@note: 析构函数
        */
		~CSceneMarkHandler();

	public:
		/**
        *@note: 事件处理
        */
		bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	public:
		/**
        *@note: 重置事件
        */
		void ResetEvent();
		
		/**
        *@note: 移除节点
        */
		void RemoveExternNode(CExternNode* pNode);

		/**  
		  * @note 安装事件监听管理器
		*/
		void SetupEventListenMgr(FeExtNode::CFeEventListenMgr* pEventListenMgr);


	protected:
		/**
        *@note: 鼠标按下
        */
        bool LeftMouseDown(float fMouseDownX, float fMouseDownY ,osgViewer::View *pViewer); 

        /**
        *@note: 鼠标左键抬起
        */
        bool LeftMouseUp(float fMouseDownX, float fMouseDownY, osgViewer::View* pViewer); 

		/**
        *@note: 鼠标右键抬起
        */
        bool RightMouseUp(float fMouseDownX, float fMouseDownY, osgViewer::View* pViewer);

		/**
		*@note: 鼠标中键抬起
		*/
		bool MouseMove(float fMouseDownX, float fMouseDownY,osgViewer::View *pViewer);

		/**
		*@note: 鼠标拖拽
		*/
		bool DragEvent(float fMouseDownX, float fMouseDownY,osgViewer::View *pViewer);

		/**
		*@note: 鼠标左键双击
		*/
		bool LeftDoubleClickEvent(float fMouseDownX, float fMouseDownY,osgViewer::View *pViewer);

		/**
		*@note: 鼠标右键双击
		*/
		bool RightDoubleClickEvent(float fMouseDownX, float fMouseDownY,osgViewer::View *pViewer);

		/**  
		  * @note 拾取节点  
		*/
		CExternNode* PickNode(osgViewer::View* pViewer);

		/**
		*@note: 通知事件监听者处理事件
		*/
		bool NotifyListenerMgr(const GUIEventAdapter& ea, GUIActionAdapter& aa);
		
	protected:
		/// 事件监听管理器
		osg::observer_ptr<CFeEventListenMgr>		m_opEventListenMgr;

		osg::observer_ptr<FeUtil::CRenderContext>	m_opRenderContext;
		osg::observer_ptr<CExComposeNode>	        m_opMarkRoot;
		CExternNode*						        m_pExternNode;
		
		//鼠标按下的X、Y位置
		float								        m_fMouseDownX;  
		float								        m_fMouseDownY;
			
		///当前视口的大小，即视口的宽和高（像素大小）
		int									        m_nWidth;
		int									        m_nHeight;

		bool								        m_bActive;
		bool								        m_bPick;
		bool								        m_bHover;
	};
}

namespace FeExtNode
{
	/**
	  * @class CExLabelMouseEventHandler
	  * @note 标牌鼠标事件处理器
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLabelMouseEventHandler : public CFeEventHandler
	{
	public:
		CExLabelMouseEventHandler();

		/**
        *@note: 事件处理
        */
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	protected:
		/**
        *@note: 鼠标按下
        */
        bool LeftMouseDown(float fMouseDownX, float fMouseDownY); 

        /**
        *@note: 鼠标左键抬起
        */
        bool LeftMouseUp(float fMouseDownX, float fMouseDownY); 

		/**
		*@note: 鼠标拖拽
		*/
		bool DragEvent(float fMouseDownX, float fMouseDownY);

		/**
		*@note: 鼠标左键双击
		*/
		bool LeftDoubleClickEvent(float fMouseDownX, float fMouseDownY);

		/**  
		  * @note 拾取节点  
		*/
		CExternNode* PickNode(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	private:
		/// 是否拖拽标牌节点
		bool                            m_bDragLabel;
		bool					        m_bActive;
		osg::Vec2                       m_vecDragPos;

		CExLabelNode*					m_pLabelNode;

		//鼠标按下的X、Y位置
		float							m_fMouseDownX;  
		float							m_fMouseDownY;
	};
}

#endif//FE_EXTERN_NODE_HANDLER_H
