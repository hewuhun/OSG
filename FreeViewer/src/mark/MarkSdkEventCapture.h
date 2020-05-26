/**************************************************************************************************
* @file MarkSdkEventCapture.h
* @note 场景标记事件监听器
* @author g00034
* @data 2017-03-14
**************************************************************************************************/
#ifndef MARK_SDK_EVENT_CAPTURE_H
#define MARK_SDK_EVENT_CAPTURE_H 1

#include <QObject>
#include <FeUtils/EventListener.h>


namespace FreeViewer
{
	 /**
      * @class CMarkEventListener
      * @note 场景标记事件监听器，用于检测SDK中标记的事件
      * @author g00034
    */
	class CMarkEventListener : public QObject, public FeUtil::CEventListener
	{
		Q_OBJECT

	public:
		/**  
			* @note 发生事件时会通过此函数通知事件监听者进行事件处理
			* @param userEvent [in] 事件对象代理
			* @return 处理完事件返回true，不处理事件返回false
		*/
		virtual bool HandleEvent(osg::ref_ptr<FeUtil::CUserEvent> userEvent)
		{
			if(userEvent->asNodeStateEvent())
			{
				emit NodeStateEventTrigger(userEvent->GetEventType());
			}
			else if(userEvent->asMouseEvent())
			{
				emit MouseEventTrigger(userEvent->GetEventType());
			}
			return true;
		}

	signals:
		/**  
		  * @note 节点状态事件处理信号
		  * @param 事件类型 FeUtil::EUserEventType
		*/
		void NodeStateEventTrigger(unsigned int);
		
		/**  
		  * @note 鼠标状态事件处理信号
		  * @param 事件类型 FeUtil::EUserEventType
		*/
		void MouseEventTrigger(unsigned int);
	};

}

#endif // MARK_SDK_EVENT_CAPTURE_H

