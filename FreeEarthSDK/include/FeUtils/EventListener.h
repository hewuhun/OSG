/**************************************************************************************************
* @file EventListener.h
* @note 场景事件监听功能相关接口
* @author c00005
* @data 2014-11-03
**************************************************************************************************/

#ifndef FE_EVENT_LISTENER_H
#define FE_EVENT_LISTENER_H

#include <FeUtils/GeneratorID.h>

#include <osgViewer/View>
#include <osgGA/GUIEventHandler>
#include <osgGA/GUIEventAdapter>

using namespace osgGA;

namespace FeUtil
{
	/**
	  * @note 用户事件类型
	  * @author c00005
	*/
	typedef enum
	{
		E_EVENT_TYPE_OBJECT_PICK = (osgGA::GUIEventAdapter::USER << 1),
		E_EVENT_TYPE_OBJECT_HOVER = (osgGA::GUIEventAdapter::USER << 2),
		E_EVENT_TYPE_OBJECT_LEAVE = (osgGA::GUIEventAdapter::USER << 3),
		E_EVENT_TYPE_OBJECT_HIGHLIGHT = (osgGA::GUIEventAdapter::USER << 4),
		E_EVENT_TYPE_OBJECT_DRAG = (osgGA::GUIEventAdapter::USER << 5),
		E_EVENT_TYPE_OBJECT_POSITION_CHANGED = (osgGA::GUIEventAdapter::USER << 6),
		E_EVENT_TYPE_OBJECT_SIZE_CHANGED = (osgGA::GUIEventAdapter::USER << 7),
		E_EVENT_TYPE_OBJECT_POSTURE_CHANGED = (osgGA::GUIEventAdapter::USER << 8),
		E_EVENT_TYPE_OBJECT_INVOKE_MENU = (osgGA::GUIEventAdapter::USER << 9),
		E_EVENT_TYPE_OBJECT_DOUBLECLICKED = (osgGA::GUIEventAdapter::USER << 10),
		E_EVENT_DEFAULT = (osgGA::GUIEventAdapter::USER << 15)
	}EUserEventType;
}

namespace FeUtil
{
	class CNodeStateEvent;
	class CMouseEvent;

	/**
	  * @class CUserEvent
	  * @brief 用户事件基类
	  * @note 用户事件基类，内部封装了用户事件的相关信息
	  * @author g00034
	*/
	class FEUTIL_EXPORT CUserEvent : public osg::Referenced
	{
	protected:
		/**  
		  * @note 用户事件基类构造函数
		  * @param nodeID [in] 发生事件的节点ID
		*/
		CUserEvent(FEID nodeID) : m_nodeID(nodeID), m_eventType(E_EVENT_DEFAULT){}
		
	public:
		virtual ~CUserEvent(){}

		/**  
		  * @note 如果对象为 CNodeStateEvent 类型则返回 this， 否则返回0
		*/
		virtual CNodeStateEvent* asNodeStateEvent(){return 0;}

		/**  
		  * @note 如果对象为 CMouseEvent 类型则返回 this， 否则返回0
		*/
		virtual CMouseEvent* asMouseEvent(){return 0;}

		/**  
		  * @note 获取事件节点的ID
		*/
		FEID GetNodeID(){return m_nodeID;}

		/**  
		  * @note 设置事件类型
		*/
		void SetEventType(unsigned int eventType){m_eventType = eventType;}
		unsigned int GetEventType(){return m_eventType;}

	protected:
		/// 事件对应的节点ID
		FEID m_nodeID;

		/// 事件类型， EUserEventType 枚举值或运算的结果
		unsigned int m_eventType;
	};

	/**
	  * @class CMouseEvent
	  * @brief 鼠标事件类
	  * @note 鼠标事件类，内部封装了鼠标事件的相关信息
	  * @author g00034
	*/
	class FEUTIL_EXPORT CMouseEvent : public CUserEvent
	{
	public:
		/**  
		  * @note 鼠标事件类类构造函数
		  * @param nodeID [in] 发生事件的节点ID
		  * @param ea [in] 事件发生时对应的事件适配器
		  * @param aa [in] 事件发生时对应的动作适配器
		*/
		CMouseEvent(FEID nodeID, const osgGA::GUIEventAdapter& ea, const osgGA::GUIActionAdapter& aa);

		/**  
		  * @note 如果对象为 CMouseEvent 类型则返回 this， 否则返回0
		*/
		virtual CMouseEvent* asMouseEvent(){return this;}

		/**  
		  * @note 获取事件发生时对应的事件/动作适配器
		*/
		const osgGA::GUIEventAdapter* GetEventAdapter();
		const osgGA::GUIActionAdapter* GetActionAdapter();
		
	private:
		const osgGA::GUIEventAdapter* m_pEA;
		const osgGA::GUIActionAdapter* m_pAA;
	};

	/**
	  * @class CNodeStateEvent
	  * @brief 节点状态事件类
	  * @note 节点状态事件类，当节点状态发生改变时会发送此事件。
	  * @author g00034
	*/
	class FEUTIL_EXPORT CNodeStateEvent : public CUserEvent
	{
	public:
		/**  
		  * @note 节点状态事件类构造函数
		  * @param nodeID [in] 发生事件的节点ID
		*/
		CNodeStateEvent(FEID nodeID) : CUserEvent(nodeID){}

		/**  
		  * @note 如果对象为 CNodeStateEvent 类型则返回 this， 否则返回0
		*/
		virtual CNodeStateEvent* asNodeStateEvent(){return this;}
	};
	
}

namespace FeUtil
{
	/**
	* @class CEventListener
	* @brief 用户使用的事件监听类
	* @note 用户可以通过继承该类实现其中Handle方法,并将其Attach到SystemService中,从而获取
	*		三维系统中产生的各种事件
			
			监听器的注册与移除接口由功能模块管理类提供
			如：要监听场景事件，则通过场景管理器提供的注册函数来注册监听器。

	* @author c00005
	*/
	class FEUTIL_EXPORT CEventListener : public osg::Referenced
	{
	public:
		CEventListener();

		virtual ~CEventListener();

	public:
		/**  
		  * @note 发生事件时会通过此函数通知事件监听者进行事件处理
		  * @param userEvent [in] 事件对象代理
		  * @return 处理完事件返回true，不处理事件返回false
		*/
		virtual bool HandleEvent(osg::ref_ptr<CUserEvent> userEvent) {return false;}
		
	};
};

namespace FeUtil
{
	/**
	* @class CFeEventHandler
	* @brief 系统事件处理
	* @note 用户可以通过继承该类,并将其添加到systemservice中,并通过handle函数处理系统的各种事件
	* @author c00005
	*/
	class FEUTIL_EXPORT CFeEventHandler: public osgGA::GUIEventHandler
	{
	public:
		CFeEventHandler();
		virtual ~CFeEventHandler();

	public:
		/**  
		  * @note 事件处理函数接口
		  * @return 处理完事件返回true，不处理事件返回false
		*/
		virtual bool handle(const GUIEventAdapter& ea, GUIActionAdapter& aa) = 0;

	public:
		/**  
		  * @brief 获取节点ID
		*/
		const FEID GetID() const;

		/**  
		  * @brief 获取事件类型
		*/
		const EUserEventType GetEventType() const;

		/**  
		  * @brief 重置事件类型
		*/
		void ResetEventType();

		/**  
		  * @brief 判断是否有事件发生
		*/
		bool IsEventHappened();

		/**  
		  * @brief 重置事件发生标记
		*/
		void ResetEventHappened();
	

	protected:
		/// 节点ID
		FEID				m_unID;

		/// 事件类型
		EUserEventType		m_unEventType;

		/// 是否有事件发生
		bool				m_bEventHappened;
	};
};

#endif //FE_EVENT_LISTENER_H