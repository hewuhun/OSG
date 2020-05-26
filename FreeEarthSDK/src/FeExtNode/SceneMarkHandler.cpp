#include <FeExtNode/SceneMarkHandler.h>

#include <osgEarth/IntersectionPicker>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

#include <FeExtNode/ExLabelNode.h>
#include <FeExtNode/ExFeatureNode.h>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExLodModelNode.h>

#include <FeExtNode/FeEventListenMgr.h>

namespace FeExtNode
{
	using namespace FeUtil;

	CSceneMarkHandler::CSceneMarkHandler( CExComposeNode* pMarkRoot, FeUtil::CRenderContext* pRender )
		:CFeEventHandler()
		,m_fMouseDownX(0.0)
		,m_fMouseDownY(0.0)
		,m_bActive(false)
		,m_bPick(false)
		,m_bHover(false)
		,m_pExternNode(NULL)
		,m_opMarkRoot(pMarkRoot)
		,m_opRenderContext(pRender)
	{

	}

	CSceneMarkHandler::~CSceneMarkHandler()
	{

	}

	bool CSceneMarkHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		//获取Viewer以及当前视口的大小
		osgViewer::View *pViewer =dynamic_cast<osgViewer::View *>(&aa);
		if (pViewer)
		{
			osg::Camera* pCamera = pViewer->getCamera();
			if (pCamera)
			{
				osg::Viewport* pViewPort = pCamera->getViewport();
				if (pViewPort)
				{
					m_nWidth = pViewPort->width();
					m_nHeight = pViewPort->height();
				}
			}
		}

		switch(ea.getEventType())
		{
			//鼠标左键按下事件响应
		case osgGA::GUIEventAdapter::PUSH:
			{
				if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
				{
					return LeftMouseDown(ea.getX(), ea.getY(), pViewer)?NotifyListenerMgr(ea, aa):false;
				}
			}
			break;

			//鼠标弹起事件响应
		case osgGA::GUIEventAdapter::RELEASE:
			{
				if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
				{
					return LeftMouseUp(ea.getX(), ea.getY(), pViewer)?NotifyListenerMgr(ea, aa):false;
				}
				else if(ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
				{
					return RightMouseUp(ea.getX(), ea.getY(), pViewer)?NotifyListenerMgr(ea, aa):false;
				}
			}
			break;

			//鼠标移动事件响应
		case osgGA::GUIEventAdapter::MOVE:
			{
				return MouseMove(ea.getX(), ea.getY(), pViewer)?NotifyListenerMgr(ea, aa):false;
			}
			break;

			//鼠标拖拽事件响应
		case osgGA::GUIEventAdapter::DRAG:
			{
				return DragEvent(ea.getX(), ea.getY(), pViewer)?NotifyListenerMgr(ea, aa):false;
			}
			break;

			//鼠标双击事件响应
		case osgGA::GUIEventAdapter::DOUBLECLICK:
			{
				if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
				{
					return LeftDoubleClickEvent(ea.getX(), ea.getY(), pViewer)?NotifyListenerMgr(ea, aa):false;
				}
			}
			break;

		default:
			break;
		}

		return false;
	}

	bool CSceneMarkHandler::NotifyListenerMgr( const GUIEventAdapter& ea, GUIActionAdapter& aa )
	{
		if(m_opEventListenMgr.valid() && m_pExternNode)
		{
			CMouseEvent* mouseEvent = new CMouseEvent(m_pExternNode->GetID(), ea, aa);
			mouseEvent->SetEventType(m_unEventType);
			m_opEventListenMgr->Notify(m_pExternNode, mouseEvent);
			return true;
		}
		return false;
	}

	bool CSceneMarkHandler::LeftMouseDown( float fMouseDownX, float fMouseDownY, osgViewer::View *pViewer)
	{
		m_fMouseDownX = fMouseDownX;
		m_fMouseDownY = fMouseDownY;

		m_pExternNode = PickNode(pViewer);
		
		if (NULL != m_pExternNode)
		{
			m_unEventType = E_EVENT_TYPE_OBJECT_PICK;
			m_unID = m_pExternNode->GetID();

			m_bEventHappened = true;
			m_bPick = true;
			return true; 
		}
		
		return false;
	}

	bool CSceneMarkHandler::LeftMouseUp( float fMouseDownX, float fMouseDownY, osgViewer::View* pViewer )
	{
		m_fMouseDownX = fMouseDownX;
		m_fMouseDownY = fMouseDownY;
		m_bPick = false;
		return false;
	}

	bool CSceneMarkHandler::RightMouseUp( float fMouseDownX, float fMouseDownY, osgViewer::View* pViewer )
	{
		m_fMouseDownX = fMouseDownX;
		m_fMouseDownY = fMouseDownY;

		if (NULL != m_pExternNode)
		{
			m_unEventType = E_EVENT_TYPE_OBJECT_INVOKE_MENU;
			m_unID = m_pExternNode->GetID();

			m_bEventHappened = true;

			return true;
		}

		return false;
	}

	bool CSceneMarkHandler::MouseMove( float fMouseDownX, float fMouseDownY ,osgViewer::View *pViewer)
	{
		if (FeMath::Equal(m_fMouseDownX, fMouseDownX) && FeMath::Equal(m_fMouseDownY, fMouseDownY))
		{
			return false;
		}

		m_fMouseDownX = fMouseDownX;
		m_fMouseDownY = fMouseDownY;

		CExternNode* pNode = PickNode(pViewer);

		if (false == m_bHover)
		{
			m_pExternNode = pNode;
			if (!m_bPick && m_pExternNode)
			{
				m_bHover = true;

				m_unEventType = E_EVENT_TYPE_OBJECT_HOVER;
				m_unID = m_pExternNode->GetID();

				m_bEventHappened = true;

				return true;
			}
		}
		else
		{
			if (m_pExternNode != pNode)
			{
				m_bHover = false;
				if (m_pExternNode)
				{
					m_unEventType = E_EVENT_TYPE_OBJECT_LEAVE;
					m_unID = m_pExternNode->GetID();

					m_bEventHappened = true;
				}
				return true;
			}
		}

		return false;
	}

	bool CSceneMarkHandler::DragEvent(float fMouseDownX, float fMouseDownY,osgViewer::View *pViewer)
	{
		if(!m_bPick)
		{
			return false;
		}
		
		m_fMouseDownX = fMouseDownX;
		m_fMouseDownY = fMouseDownY;

		if(m_pExternNode)
		{
			m_unEventType = E_EVENT_TYPE_OBJECT_DRAG;
			m_unID = m_pExternNode->GetID();

			m_bEventHappened = true;

			return true;
		}

		return false;
	}

	bool CSceneMarkHandler::LeftDoubleClickEvent( float fMouseDownX, float fMouseDownY,osgViewer::View *pViewer )
	{
		if (NULL != m_pExternNode)
		{
			m_unEventType = E_EVENT_TYPE_OBJECT_DOUBLECLICKED;
			m_unID = m_pExternNode->GetID();

			m_bEventHappened = true;
			return true; 
		}
		return false;
	}

	bool CSceneMarkHandler::RightDoubleClickEvent( float fMouseDownX, float fMouseDownY,osgViewer::View *pViewer )
	{
		return false;
	}

	CExternNode* CSceneMarkHandler::PickNode(osgViewer::View* pViewer)
	{
#if 0
		if (false == m_opMarkRoot.valid())
		{
			return NULL;
		}

		osgEarth::IntersectionPicker::Hits hits;
		osgEarth::IntersectionPicker pick(pViewer, m_opMarkRoot.get());
		pick.setTraversalMask(ACCEPT_EVENT_MASK);
		pick.pick(m_fMouseDownX, m_fMouseDownY, hits);

		for( osgEarth::IntersectionPicker::Hits::const_iterator hitItr = hits.begin(); hitItr != hits.end(); ++hitItr )
		{
			CExternNode* pExternNode = pick.getNode<CExternNode>(*hitItr);
			if (NULL != pExternNode)
			{
				return pExternNode;
			}
		}

		return NULL;
#else
		if (NULL == pViewer)
		{
			return NULL;
		}

		osgUtil::LineSegmentIntersector::Intersections intersections;
		osg::ref_ptr<osgUtil::PolytopeIntersector> ploy = new osgUtil::PolytopeIntersector(
			osgUtil::Intersector::WINDOW, 
			m_fMouseDownX-0.1, 
			m_fMouseDownY-0.1, 
			m_fMouseDownX+0.1, 
			m_fMouseDownY+0.1);
		CSceneMarkIntersectorVisitor iv(ploy.get());
		iv.setTraversalMask(ACCEPT_EVENT_MASK);
		ploy->setIntersectionLimit(osgUtil::Intersector::LIMIT_NEAREST);
		osg::Camera* pCamera = pViewer->getCamera();
		if (pCamera)
		{
			pCamera->accept(iv);
		}

		if (ploy->containsIntersections())
		{
			osgUtil::PolytopeIntersector::Intersections intersections = ploy->getIntersections();
			osgUtil::PolytopeIntersector::Intersections::iterator it = intersections.begin();

			while (it != intersections.end())
			{
				osg::NodePath nodePath = it->nodePath;
				for (osg::NodePath::iterator itr = nodePath.begin(); itr != nodePath.end(); itr++)
				{
					CExternNode* pExternNode = dynamic_cast<CExternNode*>(*itr);
					if (NULL != pExternNode)
					{
						// 所有场景节点都保存在一个CExComposeNode，所以需要过滤场景根节点，否则会每次返回根节点
						CExComposeNode* pComposeNode = dynamic_cast<CExComposeNode*>(pExternNode);
						if (pComposeNode)
						{
							continue;
						}

						return pExternNode;
					}
				}

				it++;
			}

		}

		return NULL;
#endif
	}

	void CSceneMarkHandler::ResetEvent()
	{
		m_unEventType = E_EVENT_DEFAULT;
		m_unID = -1;
		m_bEventHappened = false;
	}

	void CSceneMarkHandler::RemoveExternNode( CExternNode* pNode )
	{
		if (m_pExternNode == pNode)
		{
			m_pExternNode = NULL;
		}

		ResetEvent();
	}

	void CSceneMarkHandler::SetupEventListenMgr( FeExtNode::CFeEventListenMgr* pEventListenMgr )
	{
		m_opEventListenMgr = pEventListenMgr;
	}

}
namespace FeExtNode
{
	CSceneMarkIntersectorVisitor::CSceneMarkIntersectorVisitor( osgUtil::Intersector* intersector )
		: osgUtil::IntersectionVisitor(intersector, NULL)
	{

	}

	CSceneMarkIntersectorVisitor::~CSceneMarkIntersectorVisitor()
	{

	}

	//void CSceneMarkIntersectorVisitor::apply( osg::Billboard& geode )
	//{

	//}

	//void CSceneMarkIntersectorVisitor::apply( osg::LOD& lod )
	//{

	//}

	//void CSceneMarkIntersectorVisitor::apply( osg::PagedLOD& lod )
	//{

	//}

	//void CSceneMarkIntersectorVisitor::apply( osg::Projection& projection )
	//{

	//}

	void CSceneMarkIntersectorVisitor::apply( osg::Group& group )
	{
		osgEarth::MapNode* pMapNode = dynamic_cast<osgEarth::MapNode*>(&group);
		if (pMapNode)
		{
			return;
		}

		CExternNode* pExternNode = dynamic_cast<CExternNode*>(&group);
		if (pExternNode)
		{
			if (!enter(group))
			{
				return;
			}

			traverse(group);
			leave();
		}
		else
		{
			traverse(group);
		}
	}

}


namespace FeExtNode
{

	CExLabelMouseEventHandler::CExLabelMouseEventHandler() 
		:CFeEventHandler()
		, m_bActive(false)
		, m_bDragLabel(false)
		, m_pLabelNode(0)
	{

	}

	bool CExLabelMouseEventHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		switch(ea.getEventType())
		{
			//鼠标左键按下事件响应
		case osgGA::GUIEventAdapter::PUSH:
			{
				if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
				{
					m_pLabelNode = dynamic_cast<CExLabelNode*>(PickNode(ea, aa));
					return LeftMouseDown(ea.getX(), ea.getY());
				}
			}
			break;

			//鼠标弹起事件响应
		case osgGA::GUIEventAdapter::RELEASE:
			{
				if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
				{
					return LeftMouseUp(ea.getX(), ea.getY());
				}
			}
			break;

			//鼠标拖拽事件响应
		case osgGA::GUIEventAdapter::DRAG:
			{
				return DragEvent(ea.getX(), ea.getY());
			}
			break;

			//鼠标双击事件响应
		case osgGA::GUIEventAdapter::DOUBLECLICK:
			{
				if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
				{
					return LeftDoubleClickEvent(ea.getX(), ea.getY());
				}
			}
			break;

		default:
			break;
		}

		return false;
	}

	bool CExLabelMouseEventHandler::LeftMouseDown( float fMouseDownX, float fMouseDownY)
	{
		m_bDragLabel = false;
		m_bActive = false;
		m_fMouseDownX = fMouseDownX;
		m_fMouseDownY = fMouseDownY;

		if(m_pLabelNode)
		{
			if(m_pLabelNode->IsPtInTitle(osg::Vec2(m_fMouseDownX, m_fMouseDownY)))
			{
				m_bDragLabel = true;
				m_vecDragPos.set(m_fMouseDownX, m_fMouseDownY);
			}

			m_bActive = true;
			return true;
		}
		
		return false;
	}

	bool CExLabelMouseEventHandler::LeftMouseUp( float fMouseDownX, float fMouseDownY)
	{
		if(!m_bActive) return false;

		m_bDragLabel = false;
		m_bActive = false;

		if(abs(m_fMouseDownX - fMouseDownX) < 3 && abs(m_fMouseDownY - fMouseDownY) < 3)
		{
			if(m_pLabelNode)
			{
				return m_pLabelNode->ClickLabel(osg::Vec2(m_fMouseDownX, m_fMouseDownY), false);
			}
		}

		return false;
	}

	bool CExLabelMouseEventHandler::DragEvent( float fMouseDownX, float fMouseDownY)
	{
		if(m_bActive && m_pLabelNode)
		{
			if(m_bDragLabel)
			{
				m_pLabelNode->DragLabel(osg::Vec2(fMouseDownX, fMouseDownY), m_vecDragPos);
				m_vecDragPos.set(fMouseDownX, fMouseDownY);
            }
			return true;
		}
		return false;
	}

	bool CExLabelMouseEventHandler::LeftDoubleClickEvent( float fMouseDownX, float fMouseDownY)
	{
		if(m_pLabelNode)
		{
			m_pLabelNode->ClickLabel(osg::Vec2(fMouseDownX, fMouseDownY), true);
			return true;
		}
		return false;
	}

	CExternNode* CExLabelMouseEventHandler::PickNode( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		osgViewer::View *pViewer =dynamic_cast<osgViewer::View *>(&aa);
		osgUtil::LineSegmentIntersector::Intersections intersections;

		if (pViewer && pViewer->computeIntersections(ea,intersections))
		{
			for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
				hitr != intersections.end();
				++hitr)
			{
				osg::NodePath nodePath = hitr->nodePath;
				for (osg::NodePath::iterator itr = nodePath.begin(); itr != nodePath.end(); itr++)
				{
					CExLabelNode* pIExternNode = dynamic_cast<CExLabelNode*>(*itr);
					if (NULL != pIExternNode)
					{
						return pIExternNode;
					}
				}
			}
		}

		return 0;
	}
	
}