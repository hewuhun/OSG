#include <FeUtils/CoordConverter.h>
#include <FeShell/SceneMarkHandler.h>

namespace FeShell
{
	CSceneMarkHandle::CSceneMarkHandle()
		:osgGA::GUIEventHandler()
		,m_rpCurrentEventInform(NULL)
		,m_fMouseDownX(0.0)
		,m_fMouseDownY(0.0)
		,m_bActive(false)
		,m_bPick(false)
	{

	}

	CSceneMarkHandle::~CSceneMarkHandle()
	{

	}

	bool CSceneMarkHandle::handle
		( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* )
	{
		osgViewer::View *pViewer =dynamic_cast<osgViewer::View *>(&aa);

		if(!m_bActive)
		{
			return false;
		}

		if ( ea.getHandled())
		{
			return false;
		}

		if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
		{        
			return MouseDown(ea.getX(), ea.getY(), pViewer);
		}
		else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
		{
			aa.requestRedraw();
			m_bPick = false;
			if(ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			{
				return LeftMouseUp(ea.getX(), ea.getY());
			}

			else if(ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
			{
				return MiddleMouseUp(ea.getX(), ea.getY());
			}
		}
		else if(ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
		{
			return MouseMove(ea.getX(), ea.getY(),pViewer);
		}
		else if( ea.getEventType() == osgGA::GUIEventAdapter::DRAG)
		{
			return DragEvent(ea.getX(), ea.getY(),pViewer);
		}

		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			//aa.requestRedraw();
			m_bPick = false;
			//26 ctrl+z
			if (ea.getKey() == 26)
			{
				return Revoke();		
			}
		}

		return false;
	}

	bool CSceneMarkHandle::MouseDown( float fMouseDownX, float fMouseDownY  ,osgViewer::View *pViewer)
	{
		m_fMouseDownX = fMouseDownX;
		m_fMouseDownY = fMouseDownY;
		
		if(m_rpCurrentEventInform.valid())
		{
			osg::Vec3d vecPos(0.0, 0.0, 0.0);
			if(FeUtil::ScreenXY2DegreeLLH(m_rpCurrentEventInform->GetRenderContext(), fMouseDownX, fMouseDownY, vecPos))
			{
				osg::NodePath npNode;
				osgUtil::LineSegmentIntersector::Intersections intersections;
				npNode.push_back(m_rpCurrentEventInform.get());
				if(pViewer->computeIntersections(fMouseDownX,fMouseDownY,npNode,intersections))
				{
					m_bPick = true;
				}
			
			}
		}

		return false;
	}

	bool CSceneMarkHandle::LeftMouseUp( float fMouseDownX, float fMouseDownY )
	{
		 if(osg::equivalent(m_fMouseDownX, fMouseDownX) && osg::equivalent(m_fMouseDownY, fMouseDownY))
		 {
			 if(m_rpCurrentEventInform.valid())
			 {
				 osg::Vec3d vecPos(0.0, 0.0, 0.0);
				 if(FeUtil::ScreenXY2DegreeLLH(m_rpCurrentEventInform->GetRenderContext(), fMouseDownX, fMouseDownY, vecPos))
				 {
					 if(FeExNode::CExFeatureNode *pGeoNode =  dynamic_cast<FeExNode::CExFeatureNode *>(m_rpCurrentEventInform.get()))
						 pGeoNode->PushBackCtrlPoint(vecPos);
					 else if(dynamic_cast<FeExNode::CExPlaceNode *>(m_rpCurrentEventInform.get()) || dynamic_cast<FeExNode::CExLodModelNode *>(m_rpCurrentEventInform.get()))
						 m_rpCurrentEventInform->SetPosition(vecPos);
				 }
			 }
		 }
		
		return false;
	}


	bool CSceneMarkHandle::Revoke()
	{
			if(m_rpCurrentEventInform.valid())
			{
				FeExNode::CExFeatureNode *pGeoNode =  dynamic_cast<FeExNode::CExFeatureNode *>(m_rpCurrentEventInform.get());
				if(pGeoNode)
				{
					pGeoNode->RemoveCtrlPoint();
				}
			}

		return false;
	}

	bool CSceneMarkHandle::MiddleMouseUp( float fMouseDownX, float fMouseDownY )
	{
		if(osg::equivalent(m_fMouseDownX, fMouseDownX) && osg::equivalent(m_fMouseDownY, fMouseDownY))
		{
			if(m_rpCurrentEventInform.valid())
			{
				osg::Vec3d vecPos(0.0, 0.0, 0.0);
				if(FeUtil::ScreenXY2DegreeLLH(m_rpCurrentEventInform->GetRenderContext(), fMouseDownX, fMouseDownY, vecPos))
				{
					//m_rpCurrentEventInform->MiddleMouseUp(vecPos);
				}
			}
		}

		return false;
	}

	bool CSceneMarkHandle::MouseMove( float fMouseDownX, float fMouseDownY ,osgViewer::View *pViewer)
	{
		
		if(m_rpCurrentEventInform.valid())
		{
			osg::Vec3d vecPos(0.0, 0.0, 0.0);
			if(FeUtil::ScreenXY2DegreeLLH(m_rpCurrentEventInform->GetRenderContext(), fMouseDownX, fMouseDownY, vecPos))
			{
				//osg::NodePath npNode;
				//osgUtil::LineSegmentIntersector::Intersections intersections;
				//npNode.push_back(m_rpCurrentEventInform.get());
				//if(pViewer->computeIntersections(fMouseDownX,fMouseDownY,npNode,intersections))
				//{
				//	m_bPick = true;
				//}
				//else
				//{
				//	m_bPick = false;
				//}
				return true;
			}
		}
		//m_bPick = false;

		return false;
	}

	bool CSceneMarkHandle::DragEvent(float fMouseDownX, float fMouseDownY,osgViewer::View *pViewer)
	{
		if(!m_bPick)
		{
			return false;
		}

		if(m_rpCurrentEventInform.valid())
		{
			osg::Vec3d vecPos(0.0, 0.0, 0.0);
			if(FeUtil::ScreenXY2DegreeLLH(m_rpCurrentEventInform->GetRenderContext(), fMouseDownX, fMouseDownY, vecPos))
			{
				m_rpCurrentEventInform->SetPosition(vecPos);
				return true;
			}
		}

		return false;
	}

	bool CSceneMarkHandle::Active() const
	{
		return m_bActive;
	}

	void CSceneMarkHandle::SetActiveEventInform( FeExNode::CExternNode* pEventInform )
	{
		m_rpCurrentEventInform = pEventInform;

		m_bActive = true;
	}

	void CSceneMarkHandle::RemoveActiveEventInform()
	{
		m_rpCurrentEventInform = NULL;
		m_bActive = false;
	}


}