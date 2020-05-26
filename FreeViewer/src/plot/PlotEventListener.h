/**************************************************************************************************
* @file PlotEventListener.h
* @note 军标事件监听器
* @author w00040
* @data 2017-11-03
**************************************************************************************************/
#ifndef PLOT_EVENT_LISTENER_H
#define PLOT_EVENT_LISTENER_H 1

#include <QObject>
#include <FeUtils/EventListener.h>

namespace FreeViewer
{
	 /**
      * @class CPlotEventListener
      * @note 军标事件监听器，用于检测SDK中军标的事件
      * @author w00040
    */
	class CPlotEventListener : public QObject, public FeUtil::CEventListener
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 发生事件时会通过此函数通知事件监听者进行事件处理
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
		  * @brief 节点状态事件处理信号
		*/
		void NodeStateEventTrigger(unsigned int);
		
		/**  
		  * @brief 鼠标状态事件处理信号
		*/
		void MouseEventTrigger(unsigned int);
	};

}

#endif

