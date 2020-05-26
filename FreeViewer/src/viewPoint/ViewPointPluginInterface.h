/**************************************************************************************************
* @file ViewPointPluginInterface.h
* @note 视点管理类接口
* @author w00040
* @data 2017-3-1
**************************************************************************************************/
#ifndef VIEWPOINT_PLUGIN_INTERFACE_H
#define VIEWPOINT_PLUGIN_INTERFACE_H

#include <mainWindow/UIObserver.h>
#include <viewPoint/LocateWidget.h>

#include <FeKits/manipulator/ManipulatorUtil.h>

namespace FreeViewer
{
	/**
	  * @class CViewPointPluginInterface
	  * @brief 视点管理类
	  * @note 用于所有视点操作的管理接口
	  * @author w00040
	*/
	class CViewPointPluginInterface : public CUIObserver
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CViewPointPluginInterface(CFreeMainWindow* pMainWindow);

		/**  
		  * @brief 析构函数
		*/
		~CViewPointPluginInterface(void);

	public:
		/**  
		  * @brief 初始化界面
		*/
		void InitWidget();

		/**  
		  * @brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

	protected slots:
		/**  
		  * @brief 复位
		*/
		void SlotViewPointReset();

		/**  
		  * @brief 正北
		*/
		void SlotViewPointNorth();

		/**  
		  * @brief 垂直
		*/
		void SlotViewPointVertical();

		/**  
		  * @brief 定位
		*/
		void SlotViewPointLocal();

		/**  
		  * @brief 锁定
		*/
		void SlotViewPointLock(bool bLock);

	private:
		///视点操作器
		osg::observer_ptr<FeKit::CManipulatorManager>	m_opManipulatorManager;

		///视点搜索
		CLocateWidget*									m_pSearchWidget;
	};
}

#endif
