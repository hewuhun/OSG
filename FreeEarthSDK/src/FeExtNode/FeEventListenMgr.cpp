#include <FeUtils/logger/LoggerDef.h>
#include <FeExtNode/ExternNode.h>
#include <FeExtNode/FeEventListenMgr.h>

namespace FeExtNode
{
	FeExtNode::CFeEventListenMgr::CFeEventListenMgr()
	{
	}

	CFeEventListenMgr::~CFeEventListenMgr()
	{
	}

	bool CFeEventListenMgr::RegistEventListener( CEventListener* eventListener, CExternNode* pNodeListen/*=0*/ )
	{
		if(!eventListener) return false;

		if(pNodeListen)
		{
			/// 查找节点是否已经存在
			NODE_LITEN_LIST::iterator iterNode = FindNodeIter(pNodeListen);
			if(iterNode != m_nodeListenerList.end())
			{
				EVENT_LISTENER_LIST::iterator iterListen = std::find_if(iterNode->second.begin(), iterNode->second.end(),
					[=](CEventListener* listen){return (eventListener == listen);});

				/// 如果节点已经注册了该监听器，则不再注册
				if(iterListen == iterNode->second.end())
				{
					iterNode->second.push_back(eventListener);
				}
			}
			else
			{
				NODE_LISTEN node_listen;
				node_listen.first = pNodeListen;
				node_listen.second.push_back(eventListener);
				m_nodeListenerList.push_back(node_listen);
			}
		}
		else
		{
			m_globalListenerList.push_back(eventListener);
		}
		return true;
	}

	bool CFeEventListenMgr::UnRegistEventListener( CEventListener* eventListener, CExternNode* pNodeListen/*=0*/ )
	{
		if(!eventListener) return false;

		if(pNodeListen)
		{
			NODE_LITEN_LIST::iterator iterNode = FindNodeIter(pNodeListen);
			if(iterNode != m_nodeListenerList.end())
			{
				EVENT_LISTENER_LIST::iterator iterListen = std::find_if(iterNode->second.begin(), iterNode->second.end(),
					[=](CEventListener* listen){return (eventListener == listen);});

				if(iterListen != iterNode->second.end())
				{
					iterNode->second.erase(iterListen);
					return true;
				}
			}
		}
		else
		{
			EVENT_LISTENER_LIST::iterator iterListen = std::find_if(m_globalListenerList.begin(), m_globalListenerList.end(),
				[=](CEventListener* listen){return (eventListener == listen);});

			if(iterListen != m_globalListenerList.end())
			{
				m_globalListenerList.erase(iterListen);
				return true;
			}
		}

		return false;
	}

	bool CFeEventListenMgr::RemoveAllEventListener( CExternNode* pNodeListen/*=0*/ )
	{
		if(pNodeListen)
		{
			NODE_LITEN_LIST::iterator iterNode = FindNodeIter(pNodeListen);
			if(iterNode != m_nodeListenerList.end())
			{
				m_nodeListenerList.erase(iterNode);
				return true;
			}
		}
		else
		{
			m_globalListenerList.clear();
			return true;
		}

		return false;
	}

	bool CFeEventListenMgr::CleanEventListener()
	{
		m_globalListenerList.clear();
		m_nodeListenerList.clear();
		return true;
	}

	CFeEventListenMgr::NODE_LITEN_LIST::iterator CFeEventListenMgr::FindNodeIter( CExternNode* pNode )
	{
		NODE_LITEN_LIST::iterator iterNode = m_nodeListenerList.begin();
		while(iterNode != m_nodeListenerList.end())
		{
			if(!iterNode->first.valid())
			{
				iterNode = m_nodeListenerList.erase(iterNode);  // 删除已经无效的节点
				continue;
			}
			else if(iterNode->first.get() == pNode)
			{
				break;
			}
			iterNode++;
		}
		return iterNode;
	}

	void CFeEventListenMgr::Notify( CExternNode* pNodeListen , osg::ref_ptr<CUserEvent> userEvent)
	{
		if(pNodeListen)
		{
			NODE_LITEN_LIST::iterator iterNode = FindNodeIter(pNodeListen);
			if(iterNode != m_nodeListenerList.end())
			{
				Notify(iterNode->second, userEvent);
			}
		}

		Notify(m_globalListenerList, userEvent);
	}

	void CFeEventListenMgr::Notify( EVENT_LISTENER_LIST& eventList, osg::ref_ptr<CUserEvent> userEvent )
	{
		EVENT_LISTENER_LIST::iterator iter = eventList.begin();
		while(iter != eventList.end())
		{
			iter->get()->HandleEvent(userEvent);
			iter++;
		}
	}

}

