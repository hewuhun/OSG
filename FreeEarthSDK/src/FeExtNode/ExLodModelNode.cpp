
#include <FeUtils/RenderContext.h>
#include <FeUtils/CoordConverter.h>
#include <FeExtNode/ExLodNode.h>
#include <FeExtNode/ExternNodeEditing.h>

#include <FeExtNode/ExLodModelNode.h>


namespace FeExtNode
{
	CExLodModelNode::CExLodModelNode( CExLodModelNodeOption* opt, FeUtil::CRenderContext* pRender ) 
		: CExLodModelGroup(opt, pRender)
	{
		/// 初始化模型列表
		int modelCount = LOD_MODEL_COUNT;
		while(0 < modelCount--)
		{
			AddLodNode(new osg::Node, TRANS_ROTATE_SCALE_EFFECT);
		}
	}

	CExLodModelNode::~CExLodModelNode()
	{
	}

	bool CExLodModelNode::SetLodModelNode( E_LOD_LEVEL node_level, const std::string& nodePath, const double& dMinRange/*=-FLT_MAX*/, const double& dMaxRange/*=FLT_MAX*/ )
	{
		LodNodeInfo* pNodeInfo = GetLodNodeInfo((unsigned int)node_level);
		if(pNodeInfo)
		{
			/// 节点是否相同
			if(pNodeInfo->nodePath == nodePath && pNodeInfo->node.valid())
			{
				pNodeInfo->node->SetRange(dMinRange, dMaxRange);
				return true;
			}

			/// 新建节点
			CExLodNode* lodNode = CreateLodNode(nodePath, dMinRange, dMaxRange);

			/// 从场景中移除旧节点
			if(lodNode && RemoveLodNodeFromScene(*pNodeInfo))
			{
				/// 替换节点信息
				pNodeInfo->node = lodNode;
				pNodeInfo->nodePath = nodePath;

				/// 添加到场景
				AddLodNode2Scene(*pNodeInfo);

				return true;
			}
		}

		return false;
	}

	bool CExLodModelNode::SetLodModelNode( E_LOD_LEVEL node_level, osg::Node* pNode, const double& dMinRange/*=-FLT_MAX*/, const double& dMaxRange/*=FLT_MAX*/ )
	{
		LodNodeInfo* pNodeInfo = GetLodNodeInfo((unsigned int)node_level);
		if(pNodeInfo)
		{
			/// 新建节点
			CExLodNode* lodNode = CreateLodNode(pNode, dMinRange, dMaxRange);

			/// 从场景中移除旧节点
			if(lodNode && RemoveLodNodeFromScene(*pNodeInfo))
			{
				/// 替换节点信息
				pNodeInfo->node = lodNode;
				pNodeInfo->nodePath = "";

				/// 添加到场景
				AddLodNode2Scene(*pNodeInfo);

				return true;
			}
		}

		return false;
	}

	CExLodModelNode::LodNodeInfo CExLodModelNode::GetLodModelNodeInfo( E_LOD_LEVEL node_level )
	{
		LodNodeInfo* pNodeInfo = GetLodNodeInfo((unsigned int)node_level);
		return (pNodeInfo)? *pNodeInfo : LodNodeInfo(0);
	}

	bool CExLodModelNode::RemoveLodModel( E_LOD_LEVEL node_level )
	{
		LodNodeInfo* pNodeInfo = GetLodNodeInfo((unsigned int)node_level);
		if(pNodeInfo)
		{
			/// 从场景中移除旧节点
			if(RemoveLodNodeFromScene(*pNodeInfo))
			{
				/// 替换节点信息
				pNodeInfo->node = 0;
				pNodeInfo->nodePath = "";

				return true;
			}
		}

		return false;
	}

	bool CExLodModelNode::SetLodModelRange( E_LOD_LEVEL node_level, const double& dMinRange, const double& dMaxRange )
	{
		LodNodeInfo* pNodeInfo = GetLodNodeInfo((unsigned int)node_level);
		if(pNodeInfo && pNodeInfo->node.valid())
		{
			pNodeInfo->node->SetRange(dMinRange, dMaxRange);
			return true;
		}

		return false;
	}

	bool CExLodModelNode::GetLodModelRange( E_LOD_LEVEL node_level, double& dMinRange, double& dMaxRange )
	{
		LodNodeInfo* pNodeInfo = GetLodNodeInfo((unsigned int)node_level);
		if(pNodeInfo && pNodeInfo->node.valid())
		{
			pNodeInfo->node->GetRange(dMinRange, dMaxRange);
			return true;
		}

		return false;
	}

	CExLodModelNodeOption* CExLodModelNode::GetOption()
	{
		return dynamic_cast<CExLodModelNodeOption*>(m_rpOptions.get());
	}

	void CExLodModelNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	void CExLodModelNode::SetModelType( int nType)
	{
		GetOption()->ModelType() = nType;
	}

	int CExLodModelNode::GetModelType()
	{
		return GetOption()->ModelType();
	}


}









namespace FeExtNode
{
	CExLodModelGroup::CExLodModelGroup( CExLodModelGroupOption* opt, FeUtil::CRenderContext* pRender ) 
		: CExLodNode(opt)
		, m_opRenderContext(pRender)
	{
		m_mxScale.makeScale(1, 1, 1);
		m_mxRotate.makeRotate(0, 0, 0, 0);
		m_mxTrans.makeTranslate(0,0,0);

		m_rpMtTrans = new osg::MatrixTransform(m_mxTrans);
		m_rpMtScale = new osg::MatrixTransform(m_mxScale);
		m_rpMtRotate = new osg::MatrixTransform(m_mxRotate);
		m_rpRootNode = new osg::Group();

		m_rpMtScale->addChild(m_rpRootNode);
		m_rpMtRotate->addChild(m_rpMtScale);
		m_rpMtTrans->addChild(m_rpMtRotate);

		addChild(m_rpMtTrans);

		if(GetOption()->PositionLLHDegree().valid())
		{
			SetPositionByLLH(GetOption()->PositionLLHDegree());
		}
	}

	CExLodModelGroup::~CExLodModelGroup()
	{
	}

	CExLodNode* CExLodModelGroup::AddLodNode(osg::Node* pNode, E_CHILD_AFFECT_BY_TRANSFORM eTransformType)
	{
		LodNodeInfo lodInfo(CreateLodNode(pNode), eTransformType);

		if(AddLodNode2Scene(lodInfo))
		{
			AddLodNode2List(lodInfo);
		}

		return lodInfo.node.get();
	}

	CExLodNode* CExLodModelGroup::AddLodNode(const std::string& nodePath, E_CHILD_AFFECT_BY_TRANSFORM eTransformType)
	{
		LodNodeInfo lodInfo(CreateLodNode(nodePath), eTransformType, nodePath);

		if(AddLodNode2Scene(lodInfo))
		{
			AddLodNode2List(lodInfo);
		}
	
		return lodInfo.node.get();
	}

	bool CExLodModelGroup::RemoveLodNode(CExLodNode* lodNode)
	{
		if(!lodNode) return false;

		LodNodeInfo* pInfo = GetLodNodeInfo(lodNode);

		if(pInfo && RemoveLodNodeFromScene(*pInfo))
		{
			RemoveLodNodeFromList(*pInfo);
			return true;
		}
		
		return false;
	}

	void CExLodModelGroup::SetPositionByLLH( const osg::Vec3d& vecLLHDegree )
	{
		if(m_rpMtTrans->getMatrix().getTrans() != vecLLHDegree)
		{
			GetOption()->PositionLLHDegree() = vecLLHDegree;

			FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), vecLLHDegree, m_mxTrans);
			m_rpMtTrans->setMatrix(m_mxTrans);

			UpdateEditor();
		}
	}

	const osg::Vec3d& CExLodModelGroup::GetPositionLLH()
	{
		return GetOption()->PositionLLHDegree();
	}

	void CExLodModelGroup::SetScale( const osg::Vec3d& vecScale )
	{
		GetOption()->ScaleDegree() = vecScale;
		m_mxScale.makeScale(vecScale);
		m_rpMtScale->setMatrix(m_mxScale);
	}

	osg::Vec3d CExLodModelGroup::GetScale()
	{
		GetOption()->ScaleDegree() = m_mxScale.getScale();
		return m_mxScale.getScale();
	}

	void CExLodModelGroup::SetRotate( const osg::Vec3d& vecAngle )
	{
		GetOption()->PostureDegree() = vecAngle;
		m_mxRotate.makeRotate(
			osg::DegreesToRadians(vecAngle.x()), osg::Vec3d(1,0,0)
			, osg::DegreesToRadians(vecAngle.y()), osg::Vec3d(0,1,0)
			, osg::DegreesToRadians(vecAngle.z()), osg::Vec3d(0,0,1));

		m_rpMtRotate->setMatrix(m_mxRotate);
	}

	osg::Vec3d CExLodModelGroup::GetRotate()
	{
		osg::Vec3d vecTrans, vecScale;
		osg::Quat quatRotate, quatSo;
		m_mxRotate.decompose(vecTrans, quatRotate, vecScale, quatSo);

		double q0 = quatRotate.w();
		double q1 = quatRotate.x();
		double q2 = quatRotate.y();
		double q3 = quatRotate.z();

		osg::Vec3d vecRotate;
		vecRotate.x() = float(atan2(2*(q2*q3+q0*q1), (q0*q0-q1*q1-q2*q2+q3*q3)));
		vecRotate.y() = float(asin(-2*(q1*q3-q0*q2)));
		vecRotate.z() = float(atan2(2*(q1*q2+q0*q3), (q0*q0+q1*q1-q2*q2-q3*q3)));
		vecRotate.x() = osg::RadiansToDegrees(vecRotate.x());
		vecRotate.y() = osg::RadiansToDegrees(vecRotate.y());
		vecRotate.z() = osg::RadiansToDegrees(vecRotate.z());

		GetOption()->PostureDegree() = vecRotate;
		return vecRotate;
	}

	void CExLodModelGroup::traverse( osg::NodeVisitor& nv )
	{
		if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
		{
			/// 使用自定义中心点来进行LOD计算
			std::for_each(m_listLodNode.begin(), m_listLodNode.end(), 
				[&](LodNodeInfo& info){ if(info.node.valid()) info.node->SetCenter(GetCenter(), true); });
		}

		CExternNode::traverse(nv);
	}

	CExLodModelGroupOption* CExLodModelGroup::GetOption()
	{
		return dynamic_cast<CExLodModelGroupOption*>(m_rpOptions.get());
	}

	bool CExLodModelGroup::AddLodNode2Scene( const LodNodeInfo& info )
	{
		if(info.node.valid())
		{
			/// add to specified parent
			switch (info.transEffect)
			{
			case TRANS_EFFECT: 
				return m_rpMtTrans->addChild(info.node); 

			case TRANS_ROTATE_EFFECT:
				return m_rpMtRotate->addChild(info.node);

			case TRANS_ROTATE_SCALE_EFFECT:
				return m_rpRootNode->addChild(info.node);

			default:
				return m_rpRootNode->addChild(info.node);
			}
		}

		return false;
	}

	bool CExLodModelGroup::RemoveLodNodeFromScene( const LodNodeInfo& info )
	{
		/// add to specified parent
		switch (info.transEffect)
		{
		case TRANS_EFFECT: 
			return m_rpMtTrans->removeChild(info.node); 

		case TRANS_ROTATE_EFFECT:
			return m_rpMtRotate->removeChild(info.node);

		case TRANS_ROTATE_SCALE_EFFECT:
			return m_rpRootNode->removeChild(info.node);

		default:
			return m_rpRootNode->removeChild(info.node);
		}
	}

	CExLodModelGroup::LodNodeInfo* CExLodModelGroup::GetLodNodeInfo( const CExLodNode* pNode )
	{
		LodNodeInfo* pInfo = 0;
		std::for_each(m_listLodNode.begin(), m_listLodNode.end(), 
			[&](LodNodeInfo& info){ if(info.node == pNode){ pInfo = &info; return;} });
		return pInfo;
	}

	CExLodModelGroup::LodNodeInfo* CExLodModelGroup::GetLodNodeInfo( unsigned int nIndex )
	{
		if(nIndex >= m_listLodNode.size()) return 0;

		// 查找
		LOD_NODE_INFO_LIST::iterator iter = m_listLodNode.begin();
		while(0 < nIndex--) ++iter;
		return &*iter;
	}

	CExLodNode* CExLodModelGroup::CreateLodNode( osg::Node* pNode, const double& dMinRange/*=-FLT_MAX*/, const double& dMaxRange/*=FLT_MAX*/ )
	{
		if(!pNode) return 0;
		CExLodNode* lodNode = new CExLodNode(new CExLodNodeOption(dMinRange, dMaxRange));
		lodNode->addChild(pNode);
		return lodNode;
	}

	void CExLodModelGroup::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	FeNodeEditor::CFeNodeEditTool* CExLodModelGroup::CreateEditorTool()
	{
		return new CExModelEditing(m_opRenderContext.get(), this);
	}

	void CExLodModelGroup::DraggerStart()
	{
		m_mxTransStart = m_mxTrans;
		m_mxScaleStart = m_mxScale;
		m_mxRotateStart = m_mxRotate;
	}

	void CExLodModelGroup::DraggerUpdateMatrix( const osg::Matrix& matrix )
	{
		/// Scale
		m_mxScale = osg::Matrix::scale(matrix.getScale()) * m_mxScaleStart;

		/// Rotate
		m_mxRotate = osg::Matrix::rotate(matrix.getRotate()) * m_mxRotateStart;

		/// Trans
		m_mxTrans = m_mxTransStart * osg::Matrix::translate(matrix.getTrans());

		m_rpMtScale->setMatrix(m_mxScale);
		m_rpMtRotate->setMatrix(m_mxRotate);
		m_rpMtTrans->setMatrix(m_mxTrans);

		osg::Vec3d vecLLH;
		FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), m_mxTrans.getTrans(), vecLLH);

		// 限制模型高度无限增加
		if (vecLLH.z() > 10000000.0)
		{
			vecLLH.z() = 10000000.0;
		}
		SetPositionByLLH(vecLLH);
	}

	osg::Vec3d CExLodModelGroup::GetEditPointRotateDegree()
	{
		return GetRotate();
	}

	

}