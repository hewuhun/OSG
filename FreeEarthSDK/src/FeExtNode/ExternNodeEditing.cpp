#include <osgEarth/DPLineSegmentIntersector>
#include <FeUtils/CoordConverter.h>
#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExternNodeEditing.h>

using namespace FeNodeEditor;

namespace FeExtNode
{
	/**
	  * @class CDraggerEventHandler
	  * @brief 编辑器事件处理类
	  * @note 编辑器事件处理类，用于处理编辑器的切换，显示等操作
	  * @author g00034
	*/
	class CDraggerEventHandler : public osgGA::GUIEventHandler
	{
	private:
		/// 编辑器节点
		osg::observer_ptr<CFeNodeEditTool>  m_opEditor;

		/// 编辑节点
		osg::observer_ptr<CExternNode>      m_opEditNode;

		/// 记录鼠标按下位置
		osg::Vec2                           m_activePushPos;

		/// 是否激活编辑器
		bool                                m_bDraggerActive;

	public:
		CDraggerEventHandler(CFeNodeEditTool* pEditor, CExternNode* pEditNode)
			: m_opEditNode(pEditNode)
			, m_opEditor(pEditor)
			, m_bDraggerActive(false)
		{
		}

		virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
		{
			if(ea.getHandled()) return false;

			if(!m_opEditor.valid() || !m_opEditNode)
			{
				OSG_WARN << "CDraggerEventHandler: m_opEditor or m_opEditNode is NULL" << std::endl;
				return false;
			}

			switch(ea.getEventType())
			{
			case osgGA::GUIEventAdapter::PUSH:
				{
					m_activePushPos.set(ea.getX(), ea.getY());
					m_bDraggerActive = PickNodeAndEditor(ea, aa);

					if(!m_bDraggerActive)
					{
						//m_opEditor->HideEditor();
					}
				}break;

			case osgGA::GUIEventAdapter::RELEASE:
				{
					if(m_activePushPos.x() == ea.getX() && m_activePushPos.y() == ea.getY())
					{
						if(m_bDraggerActive)
						{
							//m_opEditor->ShowEditor();
						}
					}
				}break;

			case osgGA::GUIEventAdapter::DOUBLECLICK:
				{
					/// 切换编辑器
					if(m_bDraggerActive)
					{
						m_opEditor->SwitchEditor();
						return true;
					}
				}break;
			}

			return false;
		}

	private:
		bool PickNodeAndEditor(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
		{
			osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
			if (!view) return false;

			osgUtil::LineSegmentIntersector::Intersections intersections;

			if (view->computeIntersections(ea,intersections))
			{
				for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
					hitr != intersections.end();
					++hitr)
				{
					osg::NodePath nodePath = hitr->nodePath;
					for (osg::NodePath::iterator itr = nodePath.begin(); itr != nodePath.end(); itr++)
					{
						/** 拣选编辑节点或编辑器 */
						if ( (*itr) == m_opEditor.get() )
						{
							return true;
						}
						else if ( ((*itr) == m_opEditNode.get()) )
						{
							if((ea.getButton() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON))
							{
								return true;
							}
						}
					}
				}
			}

			return false;
		}
	};
}


namespace FeExtNode
{
	CExternNodeEditing::CExternNodeEditing(FeUtil::CRenderContext* pRender, CExternNode* pEditNode, 
		unsigned int editorTypeSupport)
		: CFeNodeEditTool(pEditNode, editorTypeSupport)
		,m_opRenderContext(pRender)
		,m_fScaleFactor(1)
		,m_bNeedRedraw(false)
	{
		addEventCallback(new CDraggerEventHandler(this, dynamic_cast<CExternNode*>(m_pEditNode)));

		ShowTransEditor();
	}

	CExternNodeEditing::~CExternNodeEditing()
	{
	}

	void CExternNodeEditing::UpdateEditingInternal()
	{
		CExternNode* node = dynamic_cast<CExternNode*>(m_pEditNode);
		if(!node) return;

		if(!node->GetVisible())
		{
			HideEditor();
			return;
		}
		else
		{
			m_bNeedRedraw = true;
		}
		
		return CFeNodeEditTool::UpdateEditingInternal();
	}

	float CExternNodeEditing::GetEditorScaleVal()
	{
		return m_fScaleFactor;
	}

	osg::Matrix CExternNodeEditing::ComputeTransMatrixByWorldXYZ( const osg::Vec3d& worldXYZ )
	{
		osg::Matrix matrixTrans;
		FeUtil::XYZ2Matrix(m_opRenderContext.get(), worldXYZ, matrixTrans);
		return matrixTrans;
	}

	void CExternNodeEditing::traverse( osg::NodeVisitor& nv )
	{
		if (m_opCurActiveDragger.valid())
		{
			if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
			{
				/// 编辑器正在编辑时不进行裁剪
				if(DRAG_DRAGGING != GetDragState())
				{
					/// 视点是否发生变化
					bool bCameraChanged = false;
					osg::Vec3 eye,center,up;
					m_opRenderContext->GetCamera()->getViewMatrixAsLookAt(eye, center, up);

					if (eye != m_eye || center != m_center || up != m_up)
					{
						m_eye = eye;
						m_center = center;
						m_up = up;
						bCameraChanged = true;
					}

					if(bCameraChanged || m_bNeedRedraw)
					{
						/// 裁剪
						CExternNode* node = dynamic_cast<CExternNode*>(m_pEditNode);
						if(node && IsNeedCull(node->GetEditPointPositionXYZ(), eye))
						{
							HideEditor();
							m_bNeedRedraw = false;
						}
						else
						{
							m_bNeedRedraw = true;
							ShowEditor();
						}
					}
				}
				
				/// 更新
				if(m_bNeedRedraw)
				{
					m_bNeedRedraw = false;

					if(IsEditorVisible())
					{
						osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>(&nv);
						float _draggerSize = m_opRenderContext->GetCamera()->getViewport()->height() / 4;
						float pixelSize = cv->pixelSize(m_opCurActiveDragger->getBound().center(),0.5f);

						if ( pixelSize!=_draggerSize )
						{
							m_fScaleFactor = pixelSize>0.0f ? _draggerSize/pixelSize : 1.0f;
							CFeNodeEditTool::UpdateEditingInternal();
						}
					}
				}
			}
		}
		CFeNodeEditTool::traverse(nv);
	}

	bool CExternNodeEditing::IsNeedCull( const osg::Vec3d& XYZ, const osg::Vec3d& eye )
	{
		osg::Vec3d eye_o_xyz = XYZ - eye;
		double dis = eye_o_xyz.length();

		if (dis > 141472)
		{
			if (eye.length2()<=0.0 || XYZ.length2()<=0.0)
			{
				return true;	
			}
			double eye_o_xyz = std::acos((XYZ*eye)/(XYZ.length()*eye.length()));
			osg::Vec3d eye2xyz = eye-XYZ;
			double angleRadian = 0.0;
			osg::Vec3d LLH;
			FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), XYZ, LLH);
			if(LLH.z() < 0) LLH.z() = 0;

			osgEarth::MapNode* mapnode = m_opRenderContext->GetMapNode() ;
			osg::BoundingSphere bs = mapnode->getBound();
			double offset = std::acos(bs.radius() / (LLH.z() + bs.radius()));
			if (eye_o_xyz >= offset)
			{
				angleRadian = osg::PI_2 + offset;
				double angle = std::acos((XYZ*eye2xyz)/(XYZ.length()*eye2xyz.length()));
				if (angle >= 0 && angle <= angleRadian) return false;
			}
			else
			{
				return false;
			}

			return true;
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

				if (picker->containsIntersections())   
				{
					osgUtil::LineSegmentIntersector::Intersections intersections = picker->getIntersections();

					if(intersections.size() == 1)
					{
						osg::Vec3d vecIntersect = intersections.begin()->getWorldIntersectPoint();
						
						if((int)vecIntersect.x() == (int)XYZ.x() &&
							(int)vecIntersect.y() == (int)XYZ.y() && 
							(int)vecIntersect.z() == (int)XYZ.z())
						{
							return false;
						}
					}

#ifdef _DEBUG
					std::cout << "Editor Culled!!!   has Intesection: " << intersections.size() << std::endl;
					std::cout << "position: " << "(" << XYZ.x() << "," << XYZ.y() << ", " << XYZ.z() << ")" << std::endl;

					std::for_each(intersections.begin(), intersections.end(), [&](const osgUtil::LineSegmentIntersector::Intersection& is){
						std::cout << "intesection: " << "(" << is.getWorldIntersectPoint().x() << "," 
							<< is.getWorldIntersectPoint().y() << ", " << is.getWorldIntersectPoint().z() << ")" << std::endl;
					});
#endif

					return true;
				}
			}

			return false;
		}
	}

}



namespace FeExtNode
{
	CExEditor2D::CExEditor2D( FeUtil::CRenderContext* pRender, CExternNode* pEditNode, 
		E_EDITOR_PLANE editorPlane /*= PLANE_XY*/, unsigned int editorTypeSupport /*= EDITOR_TRANS*/ ) 
		:CExternNodeEditing(pRender, pEditNode, editorTypeSupport)
	{
		ShowTransEditor();

		if(m_rpTransDragger.valid())
		{
			if(PLANE_XY == editorPlane)
			{
				m_rpTransDragger->ShowAxisZ(false);
			}
			else if(PLANE_XZ == editorPlane)
			{
				m_rpTransDragger->ShowAxisY(false);
			}
			else if(PLANE_YZ == editorPlane)
			{
				m_rpTransDragger->ShowAxisX(false);
			}
		}
	}
}




