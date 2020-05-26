#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExternNodeEditing.h>

namespace FeExtNode
{
	using namespace osgEarth;

	CExternNodeOption::CExternNodeOption()
		:m_strName("")
		,m_strDesc("")
		,m_bVisible(true)
		,m_bAcceptEvent(false)
		,m_vpViewPoint()
	{

	}

	CExternNodeOption::CExternNodeOption( const std::string& name )
		:m_strName(name)
		,m_strDesc("")
		,m_bVisible(true)
		,m_bAcceptEvent(false)
		,m_vpViewPoint()
	{

	}

	CExternNodeOption::~CExternNodeOption()
	{

	}
}

namespace FeExtNode
{
	CExternNode::CExternNode( CExternNodeOption* opt )
		:osg::Group()
		,m_rpOptions(opt)
	{
		m_unID = FeIDGenerator->CreateUID();

		UpdateVisibleAndEvent();
	}

	CExternNode::~CExternNode()
	{

	}

	FeUtil::FEID CExternNode::GetID()
	{
		return m_unID;
	}

	void CExternNode::SetName(const std::string& strName)
	{
		GetOption()->name() = strName;
	}

	std::string CExternNode::GetName()
	{
		return GetOption()->name();
	}

	void CExternNode::SetDescription(const std::string& strDesc)
	{
		GetOption()->description() = strDesc;
	}

	std::string CExternNode::GetDescription()
	{
		return GetOption()->description();
	}

	void CExternNode::SetVisible(const bool& bVisible)
	{
		if (GetOption()->visible() == bVisible)
		{
			return;
		}

		GetOption()->visible() = bVisible;
		
		UpdateVisibleAndEvent();

		UpdateEditor();
	}

	bool CExternNode::GetVisible()
	{
		return GetOption()->visible();
	}

	void CExternNode::SetAcceptEvent( const bool& bEvent )
	{
		if (GetOption()->acceptEvent() == bEvent)
		{
			return;
		}

		GetOption()->acceptEvent() = bEvent;

		UpdateVisibleAndEvent();
	}

	bool CExternNode::GetAcceptEvent()
	{
		return GetOption()->acceptEvent();
	}

	void CExternNode::SetViewPoint(const CFreeViewPoint& viewPoint)
	{
		GetOption()->viewPoint() = viewPoint;
	}

	FeUtil::CFreeViewPoint CExternNode::GetViewPoint()
	{
		return GetOption()->viewPoint();
	}

	CExternNodeOption* CExternNode::GetOption()
	{
		return m_rpOptions;
	}
	
	void CExternNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	void CExternNode::UpdateVisibleAndEvent()
	{
		if (GetOption()->visible())
		{
			if (GetOption()->acceptEvent())
			{
				setNodeMask(VISIBLE_MASK | ACCEPT_EVENT_MASK);
			}
			else
			{
				setNodeMask(VISIBLE_MASK);
			}
		}
		else
		{
			setNodeMask(UN_VISIBLE_EVENT_MASK);
		}
	}

	CExComposeNode* CExternNode::AsComposeNode()
	{
		return NULL;
	}
	
}
