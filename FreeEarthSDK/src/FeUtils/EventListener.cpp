#include <FeUtils/EventListener.h>

namespace FeUtil
{
	CEventListener::CEventListener()
	{

	}

	CEventListener::~CEventListener()
	{

	}

}

namespace FeUtil
{
	CMouseEvent::CMouseEvent( FEID nodeID, const osgGA::GUIEventAdapter& ea, const osgGA::GUIActionAdapter& aa ) 
		: CUserEvent(nodeID)
		, m_pEA(&ea)
		, m_pAA(&aa)
	{
	}

	const osgGA::GUIEventAdapter* CMouseEvent::GetEventAdapter()
	{
		return m_pEA;
	}

	const osgGA::GUIActionAdapter* CMouseEvent::GetActionAdapter()
	{
		return m_pAA;
	}
}

namespace FeUtil
{
	CFeEventHandler::CFeEventHandler()
		:osgGA::GUIEventHandler()
		, m_unID(-1.0)
		, m_unEventType(E_EVENT_DEFAULT)
		, m_bEventHappened(false)
	{

	}

	CFeEventHandler::~CFeEventHandler()
	{

	}

	const FEID CFeEventHandler::GetID() const
	{
		return m_unID;
	}

	const EUserEventType CFeEventHandler::GetEventType() const
	{
		return m_unEventType;
	}

	void CFeEventHandler::ResetEventType()
	{
		m_unEventType = E_EVENT_DEFAULT;
		m_unID= -1;
	}

	bool CFeEventHandler::IsEventHappened()
	{
		return m_bEventHappened;
	}

	void CFeEventHandler::ResetEventHappened()
	{
		m_bEventHappened = false;
	}

}