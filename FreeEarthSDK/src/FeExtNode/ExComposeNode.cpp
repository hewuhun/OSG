#include <FeExtNode/ExComposeNode.h>

namespace FeExtNode
{
	CExComposeNodeOption::CExComposeNodeOption()
		:CExLodNodeOption()
	{

	}

	CExComposeNodeOption::~CExComposeNodeOption()
	{
		m_vecChildren.clear();
	}
}


namespace FeExtNode
{
	CExComposeNode::CExComposeNode( CExLodNodeOption* opt )
		:CExLodNode(opt)
	{

	}

	CExComposeNode::~CExComposeNode()
	{

	}

	bool CExComposeNode::ExistNode(CExternNode* pNode)
	{
		for (int i = 0; i < GetOption()->childrens().size(); i++)
		{
			if (pNode == GetOption()->childrens().at(i))
			{
				return true;
			}
		}

		return false;
	}

	bool CExComposeNode::InsertNode(CExternNode* pNode)
	{
		if (ExistNode(pNode))
		{
			return false;
		}

		GetOption()->childrens().push_back(pNode);
		addChild(pNode);

		return true;
	}

	bool CExComposeNode::RemoveNode(CExternNode* pNode)
	{
		CExComposeNodeOption::VecExternNodes::iterator it = GetOption()->childrens().begin();
		while (it != GetOption()->childrens().end())
		{
			if (pNode == *it)
			{
				GetOption()->childrens().erase(it);
				removeChild(pNode);

				return true;
			}

			it++;
		}

		return false;
	}

	CExternNode* CExComposeNode::GetNode( const FEID& nId )
	{
		CExternNode* pFinalNode = NULL;

		CExComposeNodeOption::VecExternNodes::iterator it = GetOption()->childrens().begin();
		while (it != GetOption()->childrens().end())
		{
			CExternNode* pNode = *it;
			if (pNode && pNode->GetID() == nId)
			{
				pFinalNode = pNode;
				break;
			}

			CExComposeNode* pComposeNode = dynamic_cast<CExComposeNode*>(pNode);
			if (pComposeNode)
			{
				pFinalNode = pComposeNode->GetNode(nId);
				if (NULL != pFinalNode)
				{
					break;
				}
			}

			it++;
		}

		return pFinalNode;
	}

	CExComposeNodeOption::VecExternNodes CExComposeNode::GetChildren()
	{
		return GetOption()->childrens();
	}

	void CExComposeNode::Clear()
	{
		GetOption()->childrens().clear();
		removeChildren(0, getNumChildren());
	}

	void CExComposeNode::traverse(osg::NodeVisitor& nv)
	{
		if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
		{
			double dis = nv.getDistanceToEyePoint(GetCenter(), true);
			if (dis >= GetOption()->minRange() && dis <= GetOption()->maxRange())
			{
				CExComposeNodeOption::VecExternNodes::iterator it = GetOption()->childrens().begin();
				while (it != GetOption()->childrens().end())
				{
					if((*it).valid())
					{
						(*(it++))->accept(nv);
					}
				}
			}
		}
		else
		{
			CExLodNode::traverse(nv);
		}
	}

	CExComposeNode* CExComposeNode::AsComposeNode()
	{
		return this;
	}

	CExComposeNodeOption* CExComposeNode::GetOption()
	{
		return dynamic_cast<CExComposeNodeOption*>(m_rpOptions.get());
	}

	void CExComposeNode::UpdateVisibleAndEvent()
	{
		CExLodNode::UpdateVisibleAndEvent();

		CExComposeNodeOption::VecExternNodes vecChildren = GetOption()->childrens();
		for(CExComposeNodeOption::VecExternNodes::iterator itr = vecChildren.begin(); itr != vecChildren.end(); ++itr)
		{
			if((*itr).valid() && !(*itr)->GetVisible())
			{
				(*itr)->setNodeMask(UN_VISIBLE_EVENT_MASK);
			}
		}
	}

	void CExComposeNode::Accept( CExternNodeVisitor& nv )
	{
		if(nv.VisitEnter(*this))
		{
			CExComposeNodeOption::VecExternNodes vecChildren = GetOption()->childrens();
			for(CExComposeNodeOption::VecExternNodes::iterator itr = vecChildren.begin(); itr != vecChildren.end(); ++itr)
			{
				if((*itr).valid())
				{
					(*itr)->Accept(nv);
				}
			}
		}
		nv.VisitExit(*this);
	}

}

