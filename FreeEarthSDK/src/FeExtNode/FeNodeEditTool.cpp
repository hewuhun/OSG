#include <osg/AutoTransform>
#include <osg/ShapeDrawable>
#include <osgGA/GUIEventHandler>
#include <osgViewer/View>
#include <osgEarth/MapNode>
#include <osgEarth/GeoData>
#include <osgEarth/IntersectionPicker>
#include <FeExtNode/ExternNode.h>
#include <FeExtNode/FeEventListenMgr.h>
#include <IFeNodeEditable.h>
#include <FeNodeEditTool.h>


namespace FeNodeEditor
{
	CFeNodeEditTool::CFeNodeEditTool(IFeNodeEditable* pEditNode, unsigned int editorTypeSupport)
		:osg::Group()
		,m_pEditNode(pEditNode)
		,m_nEditorSupport(editorTypeSupport)
		,m_eDragState(DRAG_STOP)
	{
		CreateEditors(editorTypeSupport);
	}

	CFeNodeEditTool::~CFeNodeEditTool()
	{
	}

	void CFeNodeEditTool::CreateEditors(unsigned int eType)
	{
		m_rpCameraRoot = new osg::Camera;
		m_rpCameraRoot->setClearMask(GL_DEPTH_BUFFER_BIT);
		addChild(m_rpCameraRoot);
		//addEventCallback(new CDraggerEventHandler(this, m_pEditNode));

		if((eType&EDITOR_TRANS) && !m_rpTransDragger.valid())
		{
			m_rpTransDragger = new CFeTranslateDragger;
			m_rpTransDragger->setupDefaultGeometry();
			m_rpTransDragger->setHandleEvents(true);
			m_rpTransDragger->setActivationModKeyMask(osgGA::GUIEventAdapter::MODKEY_CTRL);
			m_rpTransDragger->setNodeMask(NODE_UNVISBLE_MASK);
			m_rpCameraRoot->addChild(m_rpTransDragger);

			/// 使用原点矩阵计算，得到的为全局坐标变换
			m_rpMtNode = new osg::MatrixTransform;
			m_rpTransDraggerCallback = new CExDraggerCallback(m_rpMtNode, this);
			m_rpTransDragger->addDraggerCallback(m_rpTransDraggerCallback);
		}

		if((eType&EDITOR_SCALE) && !m_rpScaleDragger.valid())
		{
			m_rpScaleDragger = new CFeScaleDragger;
			m_rpScaleDragger->setupDefaultGeometry();
			m_rpScaleDragger->setHandleEvents(true);
			m_rpScaleDragger->setActivationModKeyMask(osgGA::GUIEventAdapter::MODKEY_CTRL);
			m_rpScaleDragger->setNodeMask(NODE_UNVISBLE_MASK);
			m_rpCameraRoot->addChild(m_rpScaleDragger);

			/// 使用编辑器自身的矩阵计算，得到的为局部坐标变换
			m_rpScaleDraggerCallback = new CExDraggerCallback(m_rpScaleDragger, this);
			m_rpScaleDragger->addDraggerCallback(m_rpScaleDraggerCallback);
		}

		if((eType&EDITOR_ROTATE) && !m_rpRotateDragger.valid())
		{
			m_rpRotateDragger = new CFeRotateDragger;
			m_rpRotateDragger->setupDefaultGeometry();
			m_rpRotateDragger->setHandleEvents(true);
			m_rpRotateDragger->setActivationModKeyMask(osgGA::GUIEventAdapter::MODKEY_CTRL);
			m_rpRotateDragger->setNodeMask(NODE_UNVISBLE_MASK);
			m_rpCameraRoot->addChild(m_rpRotateDragger);

			/// 使用编辑器自身的矩阵计算，得到的为局部坐标变换
			m_rpRotateDraggerCallback = new CExDraggerCallback(m_rpRotateDragger, this);
			m_rpRotateDragger->addDraggerCallback(m_rpRotateDraggerCallback);
		}
	}

	bool CFeNodeEditTool::ShowTransEditor()
	{
		if(m_rpTransDragger.valid())
		{
			HideEditor();
			m_opCurActiveDragger = m_rpTransDragger.get();
			ShowEditor();
			return true;
		}
		return false;
	}

	bool CFeNodeEditTool::ShowScaleEditor()
	{
		if(m_rpScaleDragger.valid())
		{
			HideEditor();
			m_opCurActiveDragger = m_rpScaleDragger.get();
			ShowEditor();
			return true;
		}
		return false;
	}

	bool CFeNodeEditTool::ShowRotateEditor()
	{
		if(m_rpRotateDragger.valid())
		{
			HideEditor();
			m_opCurActiveDragger = m_rpRotateDragger.get();
			ShowEditor();
			return true;
		}
		return false;
	}

	void CFeNodeEditTool::ShowEditor()
	{
		if(!IsEditorVisible() && m_opCurActiveDragger.valid())
		{
			m_opCurActiveDragger->SetNodeVisible(true);
			UpdateEditing();
		}
	}

	void CFeNodeEditTool::HideEditor()
	{
		if(IsEditorVisible() && m_opCurActiveDragger.valid())
		{
			m_opCurActiveDragger->SetNodeVisible(false);
		}
	}

	bool CFeNodeEditTool::IsEditorVisible()
	{
		return (m_opCurActiveDragger.valid() && m_opCurActiveDragger->IsNodeVisible());
	}

	void CFeNodeEditTool::SwitchEditor()
	{
		if(!IsEditorVisible()) return;

		/// 按照 平移 --> 缩放 --> 旋转 的顺序切换
		if(m_opCurActiveDragger.get() == m_rpTransDragger.get())
		{
			if(!ShowScaleEditor())
			{
				ShowRotateEditor();
			}
		}
		else if(m_opCurActiveDragger.get() == m_rpScaleDragger.get())
		{
			if(!ShowRotateEditor())
			{
				ShowTransEditor();
			}
		}
		else
		{
			if(!ShowTransEditor())
			{
				ShowScaleEditor();
			}
		}
	}

	void CFeNodeEditTool::UpdateEditing()
	{
		UpdateEditingInternal();
	}

	void CFeNodeEditTool::UpdateEditingInternal()
	{
		if(!m_pEditNode) return;

		/// 平移矩阵
		osg::Matrix matrixTrans = ComputeTransMatrixByWorldXYZ(m_pEditNode->GetEditPointPositionXYZ());

		/// 旋转矩阵
		osg::Matrix matrixRotate(osg::Matrix::rotate(
			osg::DegreesToRadians(m_pEditNode->GetEditPointRotateDegree().x()), osg::X_AXIS,
			osg::DegreesToRadians(m_pEditNode->GetEditPointRotateDegree().y()), osg::Y_AXIS,
			osg::DegreesToRadians(m_pEditNode->GetEditPointRotateDegree().z()), osg::Z_AXIS) );

		/// 缩放矩阵
		float scale = GetEditorScaleVal();
		osg::Matrix matrixScale(osg::Matrix::scale(scale, scale, scale));

		/// Trans
		if(m_rpTransDragger.valid() && m_rpTransDragger->IsNodeVisible())
		{
			m_rpTransDragger->setMatrix(matrixScale * matrixRotate * matrixTrans);
		}

		/// Scale
		if(m_rpScaleDragger.valid() && m_rpScaleDragger->IsNodeVisible())
		{
			m_rpScaleDragger->setMatrix(matrixScale * matrixRotate * matrixTrans);
		}

		/// Rotate
		if(m_rpRotateDragger.valid() && m_rpRotateDragger->IsNodeVisible())
		{
			m_rpRotateDragger->setMatrix(matrixScale * matrixRotate * matrixTrans);
		}
	}

	void CFeNodeEditTool::SetupEventListenMgr( FeExtNode::CFeEventListenMgr* pEventListenMgr )
	{
		m_opEventListenMgr = pEventListenMgr;
	}

	void CFeNodeEditTool::DraggerStart()
	{
		m_eDragState = DRAG_START;

		if(m_pEditNode)
		{
			m_pEditNode->DraggerStart();
		}
	}

	void CFeNodeEditTool::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		m_eDragState = DRAG_DRAGGING;

		if(m_pEditNode)
		{
			m_pEditNode->DraggerUpdateMatrix(matrix);
			NotifyListenerMgr();
		}
	}

	void CFeNodeEditTool::DraggerEnd()
	{
		m_eDragState = DRAG_STOP;

		if(m_pEditNode)
		{
			m_pEditNode->DraggerEnd();
		}
	}

	void CFeNodeEditTool::NotifyListenerMgr()
	{
		if(!m_opEventListenMgr.valid()) return;

		FeExtNode::CExternNode* pNode = dynamic_cast<FeExtNode::CExternNode*>(m_pEditNode);
		if(pNode)
		{
			CNodeStateEvent* stateEvent = new CNodeStateEvent(pNode->GetID());

			if(m_rpRotateDragger.valid() && m_rpRotateDragger == m_opCurActiveDragger.get())
				stateEvent->SetEventType(FeUtil::E_EVENT_TYPE_OBJECT_POSTURE_CHANGED);
			else if(m_rpScaleDragger.valid() && m_rpScaleDragger == m_opCurActiveDragger.get())
				stateEvent->SetEventType(FeUtil::E_EVENT_TYPE_OBJECT_SIZE_CHANGED);
			else if(m_rpTransDragger.valid() && m_rpTransDragger == m_opCurActiveDragger.get())
				stateEvent->SetEventType(FeUtil::E_EVENT_TYPE_OBJECT_POSITION_CHANGED);

			m_opEventListenMgr->Notify(pNode, stateEvent);
		}
	}

}


namespace FeNodeEditor
{
	CExDraggerCallback::CExDraggerCallback(osg::MatrixTransform* transform, CFeNodeEditTool* pEditTool,
		int handleCommandMask/*= osgManipulator::DraggerTransformCallback::HANDLE_ALL*/)
		: osgManipulator::DraggerTransformCallback(transform, handleCommandMask)
		, m_opEditTool(pEditTool)
	{
	}

	bool CExDraggerCallback::receive(const osgManipulator::MotionCommand& command)
	{
		if (!_transform) return false;

		switch (command.getStage())
		{
		case osgManipulator::MotionCommand::START:
			{
				if(m_opEditTool.valid())
				{
					/// 通知节点开始编辑
					m_opEditTool->DraggerStart();
				}

				// Save the current matrix
				_startMotionMatrix = _transform->getMatrix();

				// Get the LocalToWorld and WorldToLocal matrix for this node.
				osg::NodePath nodePathToRoot;
				osgManipulator::computeNodePathToRoot(*_transform,nodePathToRoot);
				_localToWorld = osg::computeLocalToWorld(nodePathToRoot);
				_worldToLocal = osg::Matrix::inverse(_localToWorld);

				return true;
			}
		case osgManipulator::MotionCommand::MOVE:
			{
				// Transform the command's motion matrix into local motion matrix.
				osg::Matrix localMotionMatrix = _localToWorld * command.getWorldToLocal()
					* command.getMotionMatrix()
					* command.getLocalToWorld() * _worldToLocal;

				// Transform by the localMotionMatrix
				//_transform->setMatrix(localMotionMatrix * _startMotionMatrix);

				if(m_opEditTool.valid())
				{
					/// 通知节点编辑器变换量
					m_opEditTool->DraggerUpdateMatrix(localMotionMatrix);
				}

				return true;
			}
		case osgManipulator::MotionCommand::FINISH:
			{
				if(m_opEditTool.valid())
				{
					/// 通知节点结束编辑
					m_opEditTool->DraggerEnd();
				}
				return true;
			}
		}
		
		return true;
	}

}


namespace FeNodeEditor
{
	CFeEditPoint::CFeEditPoint( osgEarth::MapNode* mapNode, IFeEditPointEventReceiver* receiver, int pointIndex )
		: m_pEventReceiver(receiver)
		, m_opMapNode(mapNode)
		, m_pointIndex(pointIndex)
		, m_bActive(false)
	{
		setNumChildrenRequiringEventTraversal( 1 );

		this->addChild(CreateSperePoint());

		getOrCreateStateSet()->setMode(GL_DEPTH_TEST, 
			osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
		getOrCreateStateSet()->setRenderBinDetails(99, "RenderBin");
	}

	void CFeEditPoint::SetColor(const osg::Vec4 vecColor)
	{
		m_shapeDrawable->setColor(vecColor);
	}

	void CFeEditPoint::SetPosition( const osgEarth::GeoPoint& position)
	{
		if(!m_pEventReceiver || !m_opMapNode.valid())
		{
			return;
		}

		osg::Matrixd matrix;
		osgEarth::GeoPoint mapPoint( position );
		mapPoint = mapPoint.transform( m_opMapNode->getMapSRS() );
		if (mapPoint.makeAbsolute( m_opMapNode->getTerrain() ))
		{
			mapPoint.createLocalToWorld( matrix );
			setMatrix( matrix );	
		}
	}

	void CFeEditPoint::traverse(osg::NodeVisitor& nv)
	{
		if (nv.getVisitorType() == osg::NodeVisitor::EVENT_VISITOR)
		{
			osgGA::EventVisitor* ev = static_cast<osgGA::EventVisitor*>(&nv);
			for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
				itr != ev->getEvents().end();
				++itr)
			{
				osgGA::GUIEventAdapter* ea = dynamic_cast<osgGA::GUIEventAdapter*>(itr->get());
				if ( ea && handle(*ea, *(ev->getActionAdapter())))
					ea->setHandled(true);
			}
		}
		osg::MatrixTransform::traverse(nv);
	}

	bool CFeEditPoint::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		if (ea.getHandled()) return false;

		osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
		if (!view) return false;

		if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
		{
			m_activePushPos.set(ea.getX(), ea.getY());

			osgEarth::IntersectionPicker picker( view, this );
			osgEarth::IntersectionPicker::Hits hits;

			if ( picker.pick( ea.getX(), ea.getY(), hits ) )
			{
				m_bActive = true;
			}
		}
		else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
		{
			if(m_activePushPos.x() == ea.getX() && m_activePushPos.y() == ea.getY())
			{
				if(m_bActive && m_pEventReceiver)
				{
					m_pEventReceiver->ActiveEditPoint(*this);
					m_bActive = false;
				}
			}
		}
	
		return false;
	}

	osg::Node* CFeEditPoint::CreateSperePoint()
	{
		//Build the handle
		osg::Sphere* shape = new osg::Sphere(osg::Vec3(0,0,0), 1.0f);   
		osg::Geode* geode = new osg::Geode();
		m_shapeDrawable = new osg::ShapeDrawable( shape );    
		m_shapeDrawable->setDataVariance( osg::Object::DYNAMIC );
		geode->addDrawable(m_shapeDrawable.get());          

		geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

		osg::MatrixTransform* _scaler = new osg::MatrixTransform;
		_scaler->setMatrix( osg::Matrixd::scale( 6, 6, 6 ));
		_scaler->addChild( geode );

		osg::AutoTransform* at = new osg::AutoTransform;
		at->setAutoScaleToScreen( true );
		at->addChild( _scaler );

		return at;
	}
	
}


