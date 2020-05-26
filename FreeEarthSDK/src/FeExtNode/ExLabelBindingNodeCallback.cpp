#include <FeUtils/CoordConverter.h>
#include <FeUtils/RenderContext.h>

#include <FeExtNode/ExLabelNode.h>

#include <ExLabelBindingNodeCallback.h>


namespace FeExtNode
{
	CExLabelBindingNodeCallback::CExLabelBindingNodeCallback( FeUtil::CRenderContext* pRender )
		: osg::NodeCallback()
	    , m_opRenderContext(pRender)
	{

	}

	CExLabelBindingNodeCallback::~CExLabelBindingNodeCallback( void )
	{
		if(m_opBindingNode.valid())
		{
			m_opBindingNode->removeUpdateCallback(this);
		}
	}

	void CExLabelBindingNodeCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(node && m_opBindingNode.valid() && m_opLabelNode.valid() && (node == m_opBindingNode.get()))
		{
			// 检测绑定节点位置是否发生变化

			osg::Vec3d vecXYZ, vecLLH;
			CExternNode* pNode = dynamic_cast<CExternNode*>(node);

			if(pNode)
			{
				vecXYZ = pNode->GetEditPointPositionXYZ();
			}
			else
			{
				osg::MatrixList mtList = node->getWorldMatrices();
				if(!mtList.empty())
				{
					vecXYZ = mtList.at(0).getTrans();
				}
			}

			if(FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(), vecXYZ, vecLLH))
			{
				if(m_opLabelNode->GetBindingPointLLH() != vecLLH)
				{
					/// 重新设置绑定位置
					m_opLabelNode->SetBindingPointLLH(vecLLH);
				}
			}
		}

		traverse(node, nv);
	}

	bool CExLabelBindingNodeCallback::AddLabelBindNode( CExLabelNode* pLabel, osg::Node* pBindNode )
	{
		if(!pLabel || !pBindNode)
		{
			return false;
		}

		if(m_opBindingNode.valid())
		{
			if(m_opBindingNode.get() != pBindNode)
			{
				m_opBindingNode->removeUpdateCallback(this);
			}
		}
		
		m_opBindingNode = pBindNode;
		m_opBindingNode->addUpdateCallback(this);
		m_opLabelNode = pLabel;

		return true;
	}

	bool CExLabelBindingNodeCallback::RemoveBindNode( osg::Node* pBindNode )
	{
		if(m_opBindingNode.valid())
		{
			if(m_opBindingNode.get() == pBindNode)
			{
				m_opBindingNode->removeUpdateCallback(this);
				m_opBindingNode = 0;
				return true;	
			}
		}
		return false;
	}

}
