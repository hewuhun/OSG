/**************************************************************************************************
* @file FeEventListenMgr.h
* @note 事件监听管理器
* @author g00034
* @data 2016-12-23
**************************************************************************************************/

#ifndef FE_EVENT_LISTEN_MGR_H
#define FE_EVENT_LISTEN_MGR_H

#include <FeExtNode/Export.h>
#include <FeUtils/EventListener.h>

namespace FeExtNode
{
	using namespace FeUtil;
	class CExternNode;

	/**
	  * @class CFeEventListenMgr
	  * @brief 事件监听管理器
	  * @note 用于管理用户注册的事件监听器
	  * @author g00034
	*/
    class FEEXTNODE_EXPORT CFeEventListenMgr : public osg::Referenced
    {
    public:
		/**  
		  * @note 事件监听管理器的构造函数
		*/
        CFeEventListenMgr();
		
		/**  
		  * @note 事件监听管理器的析构函数
		*/
		virtual ~CFeEventListenMgr();

	public:
		/**  
		  * @note 注册指定节点的事件监听器
		  * @param eventListener [in] 事件监听器
		  * @param pNodeListen [in] 要监听的节点，默认不指定节点，则注册全局监听器
		  * @return 是否注册成功
		*/
		bool RegistEventListener(CEventListener* eventListener, CExternNode* pNodeListen=0);

		/**  
		  * @note 移除指定节点的事件监听器
		  * @param eventListener [in] 事件监听器
		  * @param pNodeListen [in] 要监听的节点，默认不指定节点，则移除全局监听器
		  * @return 是否移除成功
		*/
		bool UnRegistEventListener(CEventListener* eventListener, CExternNode* pNodeListen=0);

		/**  
		  * @note 移除指定节点的所有事件监听器
		  * @param pNodeListen [in] 要监听的节点，默认不指定节点，则移除所有全局监听器
		  * @return 是否移除成功
		*/
		bool RemoveAllEventListener(CExternNode* pNodeListen=0);
		
		/**  
		  * @note 移除所有监听器
		  * @return 是否移除成功
		*/
		bool CleanEventListener();

		/**  
		  * @note 通知监听器，节点发生变化
		*/
		void Notify(CExternNode* pNodeListen, osg::ref_ptr<CUserEvent> userEvent);

        typedef std::vector<osg::ref_ptr<CEventListener> > EVENT_LISTENER_LIST;
		typedef std::pair<osg::observer_ptr<CExternNode>, EVENT_LISTENER_LIST> NODE_LISTEN;
		typedef std::vector<NODE_LISTEN> NODE_LITEN_LIST;

	private:
		/**  
		  * @note 查找指定节点的迭代器
		  * @return 找到返回节点对应迭代器，否则返回末尾迭代器
		*/
		NODE_LITEN_LIST::iterator FindNodeIter(CExternNode* pNode);

		/**  
		  * @note 通知 eventList 列表中的每一个监听器 pNodeListen 节点发生变化
		*/
		void Notify(EVENT_LISTENER_LIST& eventList, osg::ref_ptr<CUserEvent> userEvent);

	protected:
		/// 全局监听器
		EVENT_LISTENER_LIST m_globalListenerList;

		/// 指定节点监听器
		NODE_LITEN_LIST     m_nodeListenerList;
		
	};
}

#endif//FE_EVENT_LISTEN_MGR_H
