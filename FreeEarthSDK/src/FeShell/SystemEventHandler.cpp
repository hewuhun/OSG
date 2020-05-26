#include <FeShell/SystemEventHandler.h>

namespace FeShell
{
	using namespace FeUtil;

	CSystemHandler::CSystemHandler(CFreeEarth* pFreeEarth)
		:m_opFreeEarth(pFreeEarth)
		,m_pEventUser(NULL)
	{

	}

	CSystemHandler::~CSystemHandler()
	{
		m_ltEventList.clear();
	}

	bool CSystemHandler::handle(const GUIEventAdapter& ea, GUIActionAdapter& aa)
	{
		GUIEventAdapter::EventType unUserEvent = ea.getEventType();
		FEID unID = -1;
		bool bHandled = false;
		osgViewer::View * view = dynamic_cast<osgViewer::View*>(&aa);
		if (!view)
		{
			return false;
		}
		FeEventHandlerList::iterator it = m_ltEventList.begin();
		while (it != m_ltEventList.end())
		{
			if (NULL == (*it))
			{
				it++;
				continue;
			}

			bHandled = (*it)->handle(ea, aa);
			if ((*it)->IsEventHappened())
			{
				(*it)->ResetEventHappened();
				unUserEvent = (GUIEventAdapter::EventType)(unUserEvent | (*it)->GetEventType());
				unID = (*it)->GetID();
				(*it)->ResetEventType();
			}

			if (bHandled)
			{	
				break;
			}

			it++;
		}

		//用户是否有事件处理
		bool bUserHandler = false;
		if (NULL != m_pEventUser)
		{
			osg::ref_ptr<GUIEventAdapter> rpUserEventAdapter = new GUIEventAdapter(ea);
			rpUserEventAdapter->setEventType(unUserEvent);
			//m_pEventUser->SetID(unID);
			//bUserHandler = m_pEventUser->handleEvent(*rpUserEventAdapter, view);
		}

		//如果用户和本程序有使用事件处理，
		if (bHandled || bUserHandler)
		{
			return true;
		}
		
		return false;
	}

	void CSystemHandler::AttachUserEvent(CEventListener* pEvent)
	{
		m_pEventUser = pEvent;
	}

	void CSystemHandler::PushFbEventHandler(CFeEventHandler* pHander)
	{
		if (NULL != pHander)
		{
			m_ltEventList.push_back(pHander);
		}
	}

	void CSystemHandler::RemoveFbEventHandler(CFeEventHandler* pHander)
	{
		if (NULL != pHander)
		{
			m_ltEventList.remove(pHander);
		}
	}

}
