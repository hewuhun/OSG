#include <FeExtNode/ExLabelCullEventHandler.h>

#include <FeUtils/RenderContext.h>
#include <FeUtils/CoordConverter.h>

#include <osgEarth/DPLineSegmentIntersector>

#include <FeExtNode/ExLabelNode.h>

namespace FeExtNode
{

	bool CExLabelCullEventHandler::handle( const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa )
	{
		if (!m_view.valid())
		{
			m_view = dynamic_cast<osgViewer::View*>(aa.asView());
		}

		osg::Camera* pCamera = m_view->getCamera();
		if (NULL == pCamera)
		{
			return false;
		}

		osg::Viewport* pViewport = pCamera->getViewport();
		if (NULL == pViewport)
		{
			return false;
		}

		// 放缩比率变化
		if (m_bfirstFrame)
		{
			m_viewportWidth = pViewport->width();
			m_viewportHeight = pViewport->height();
			m_bfirstFrame = false;
			return false;
		}

		// 视点变化
		osg::Vec3 eye,center,up;
		pCamera->getViewMatrixAsLookAt(eye, center, up);

		bool bIsCameraPosChange = false;
		if (eye != m_eye || center != m_center || up != m_up)
		{
			m_eye = eye;
			m_center = center;
			m_up = up;
			bIsCameraPosChange = true;
		}

		double sw = double(pViewport->width()) / double(m_viewportWidth);
		double sh = double(pViewport->height()) / double(m_viewportHeight);

		bool IsNeedScale = false;
		if (std::abs(sw-1.0)>0.000001 || std::abs(sh-1.0)>0.000001)
		{
			m_viewportWidth = pViewport->width();
			m_viewportHeight = pViewport->height();
			IsNeedScale = true;
		}

		m_mutex.lock();

		for (BillBoardNodeList::iterator itr = m_NodeList.begin(); itr!=m_NodeList.end(); itr++)
		{
			if (!itr->valid() || (!IsNeedScale && !bIsCameraPosChange && !(*itr)->IsNeedRedraw())) 
			{
				continue;
			}

			/// 这里不能使用 SetVisible 设置Label的显隐，否则外部使用 SetVisible 隐藏节点就无效了
			/// 隐藏了则不处理
			if(!itr->get()->GetVisible())
			{
				continue;
			}

			// 是否需要重绘
			bool bNeedRedraw = (*itr)->IsNeedRedraw();

			// 需要放缩
			if (IsNeedScale && !bNeedRedraw) 
			{
				bNeedRedraw = true;
			}

			// 如果视点变化，进行视口裁切
			if (bIsCameraPosChange || (*itr)->IsNeedRedraw())
			{
				osg::Vec3d LLH = (*itr)->GetBindingPointLLH();
				
				/// Fix 地形加载导致位置被裁剪(g00034 2017.03.21)
				osg::Vec3d posLLH = LLH;
				FeUtil::DegreeLL2LLH(m_opRenderContext.get(), posLLH);
				//if(posLLH.z() > LLH.z())
				if((*itr)->IsClampToTerrain())
				{
					posLLH += osg::Vec3d(0,0,0.01);
					(*itr)->SetBindingPointLLH(posLLH);
					LLH = posLLH;
				}
				

				osg::Vec3d XYZ;
				osg::Vec2d vecScreen;
				FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), LLH, XYZ);
				FeUtil::ConvertLocalWorldCoordToScreen(m_opRenderContext.get(), XYZ, vecScreen);
				
				if ((m_viewportWidth >= vecScreen.x() && vecScreen.x() > 0 
					&& m_viewportHeight >= vecScreen.y() && vecScreen.y() > 0) // 视口裁剪
					//&& IsNotCull(XYZ, eye)) // 深度裁切
					&& IsNotCull((*itr).get(), eye)) // 深度裁切
				{
					//(*itr)->SetVisible(true);
					itr->get()->setNodeMask(0xFF);
					bNeedRedraw = true;
				}
				else
				{
					//(*itr)->SetVisible(false);
					itr->get()->setNodeMask(0x00);
					bNeedRedraw = false;
				}
			}

			if(bNeedRedraw)
			{
				(*itr)->Redraw();
			}
		}

		m_mutex.unlock();

		return false;
	}

	CExLabelCullEventHandler::CExLabelCullEventHandler(FeUtil::CRenderContext* pRender)
		: m_bfirstFrame(false)
		, m_opRenderContext(pRender)
	{

	}

	CExLabelCullEventHandler::~CExLabelCullEventHandler()
	{

	}

	bool CExLabelCullEventHandler::IsNotCull( const osg::Vec3d& XYZ, const osg::Vec3d& eye )
	{
		osg::Vec3d eye_o_xyz = XYZ - eye;
		double dis = eye_o_xyz.length();

		if (dis > 141472)
		{
			if (eye.length2()<=0.0 || XYZ.length2()<=0.0)
			{
				return false;	
			}
			double eye_o_xyz = std::acos((XYZ*eye)/(XYZ.length()*eye.length()));
			osg::Vec3d eye2xyz = eye-XYZ;
			double angleRadian = 0.0;
			osg::Vec3d LLH;
			FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), XYZ, LLH);
			osgEarth::MapNode* mapnode = m_opRenderContext->GetMapNode() ;
			osg::BoundingSphere bs = mapnode->getBound();
			double offset = std::acos(bs.radius() / (LLH.z() + bs.radius()));
			if (eye_o_xyz >= offset)
			{
				angleRadian = osg::PI_2 + offset;
				double angle = std::acos((XYZ*eye2xyz)/(XYZ.length()*eye2xyz.length()));
				if (angle >= 0 && angle <= angleRadian) return true;
			}
			else
			{
				return true;
			}
			return false;
		}
		else
		{
			osg::ref_ptr< osgEarth::DPLineSegmentIntersector > picker =
				new osgEarth::DPLineSegmentIntersector(osgUtil::Intersector::MODEL, eye, XYZ);    
			picker->setIntersectionLimit( osgUtil::Intersector::LIMIT_ONE ); 

			osgUtil::IntersectionVisitor iv(picker.get());

			//tips:
			//1.need to set traversalMask for terrainNodes previously. when iv accept terrain, we check traversalMask to ignore those which don`t care 
			//2.calculating absolute tile extents for getting valuable terrainNodes.Inherited IntersectionVisitor, reimplement apply() method to improve visitor efficiency
			osgEarth::MapNode* mapnode = m_opRenderContext->GetMapNode() ;
			if (mapnode	&& mapnode->getTerrain() && mapnode->getTerrain()->getGraph())
			{
				osgEarth::Terrain* terrain = mapnode->getTerrain();
				mapnode->getTerrain()->getGraph()->accept(iv);
				if (!picker->containsIntersections())   
				{               
					return true;          
				}
			}
			return false;
		}
	}

	bool CExLabelCullEventHandler::IsNotCull( CExLabelNode* pLabelNode, const osg::Vec3d& eye )
	{
		if(!pLabelNode) return false;

		osg::Vec3d XYZ;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), pLabelNode->GetBindingPointLLH(), XYZ);
		
		osg::Vec3d eye_o_xyz = XYZ - eye;
		double dis = eye_o_xyz.length();

		if (dis > 141472)
		{
			if (eye.length2()<=0.0 || XYZ.length2()<=0.0)
			{
				return false;	
			}

			if(pLabelNode->IsClampToTerrain())
			{
				/// 眼睛到目标点的向量v1与地心到目标点的向量v2的夹角，如果大于90度则看得见，小于90度则看不见
				double angle = std::acos((XYZ*eye_o_xyz)/(XYZ.length()*eye_o_xyz.length()));
				if(angle >= osg::PI_2)
				{
					return true;
				}
			}
			else
			{
				double eye_o_xyz = std::acos((XYZ*eye)/(XYZ.length()*eye.length()));
				osg::Vec3d eye2xyz = eye-XYZ;
				double angleRadian = 0.0;
				osgEarth::MapNode* mapnode = m_opRenderContext->GetMapNode() ;
				osg::BoundingSphere bs = mapnode->getBound();
				double offset = std::acos(bs.radius() / (pLabelNode->GetBindingPointLLH().z() + bs.radius()));
				if (eye_o_xyz >= offset)
				{
					angleRadian = osg::PI_2 + offset;
					double angle = std::acos((XYZ*eye2xyz)/(XYZ.length()*eye2xyz.length()));
					if (angle >= 0 && angle <= angleRadian) return true;
				}
				else
				{
					return true;
				}
			}

			return false;
		}
		else
		{
			osg::ref_ptr< osgEarth::DPLineSegmentIntersector > picker =
				new osgEarth::DPLineSegmentIntersector(osgUtil::Intersector::MODEL, eye, XYZ);    
			picker->setIntersectionLimit( osgUtil::Intersector::LIMIT_ONE ); 

			osgUtil::IntersectionVisitor iv(picker.get());

			//tips:
			//1.need to set traversalMask for terrainNodes previously. when iv accept terrain, we check traversalMask to ignore those which don`t care 
			//2.calculating absolute tile extents for getting valuable terrainNodes.Inherited IntersectionVisitor, reimplement apply() method to improve visitor efficiency
			osgEarth::MapNode* mapnode = m_opRenderContext->GetMapNode() ;
			if (mapnode	&& mapnode->getTerrain() && mapnode->getTerrain()->getGraph())
			{
				osgEarth::Terrain* terrain = mapnode->getTerrain();
				mapnode->getTerrain()->getGraph()->accept(iv);
				if (!picker->containsIntersections())   
				{               
					return true;          
				}
			}
			return false;
		}
	}

	bool CExLabelCullEventHandler::AddLabel( CExLabelNode* pNode )
	{
		BillBoardNodeList::iterator iter = std::find(m_NodeList.begin(), m_NodeList.end(), pNode);
		if(iter == m_NodeList.end())
		{
			m_NodeList.push_back(pNode);
			return true;
		}

		return false;
	}

	bool CExLabelCullEventHandler::RemoveLabel( CExLabelNode* pNode )
	{
		BillBoardNodeList::iterator iter = std::find(m_NodeList.begin(), m_NodeList.end(), pNode);
		if(iter != m_NodeList.end())
		{
			m_NodeList.remove(pNode);
			return true;
		}

		return false;
	}

	unsigned int CExLabelCullEventHandler::GetValidLabelNum()
	{
		unsigned int labelNum = 0;
		BillBoardNodeList::iterator iter = m_NodeList.begin();
		while(iter != m_NodeList.end())
		{
			if(iter->valid())
			{
				++labelNum;
			}
			else
			{
				m_NodeList.erase(iter);
			}
			iter++;
		}
		return labelNum;
	}

}
