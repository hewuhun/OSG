/**************************************************************************************************
* @file HelpMenu.h
* @note 帮助菜单
* @author w00040
* @data 2018-8-17
**************************************************************************************************/
#ifndef VIEW_HELP_MENU_H
#define VIEW_HELP_MENU_H 1

#include <mainWindow/UIObserver.h>
#include <help/AboutWidget.h>

namespace FreeViewer
{
	class CFreeDialog;

	/**
	* @class CHelpMenu
	* @note 帮助菜单
	* @author w00040
	*/
	class CHelpMenu : public CUIObserver
	{
		Q_OBJECT 

	public:
		/**  
		* @note 构造函数 
		*/
		CHelpMenu( CFreeMainWindow* pMainWindow);

		/**  
		* @note 析构函数 
		*/
		~CHelpMenu();

	protected:
		/**
		*note：初始化窗口
		*/
		void InitWidget();

	private slots:
		/**
		*note：关于
		*/
		void SlotAbout();

		/**  
		  * @brief 打开用户手册
		*/
		void SlotOpenHelp();

		/**  
		  * @brief 在线更新
		*/
		void SlotOnlineUpdate();

		/**  
		  * @note 访问官网
		*/
		void SlotWebsite();
	private:
		/// 帮助对话框
		CAboutWidget*	m_pAboutDlg;

		/// 帮助按钮动作
		QAction*		m_pActionAbout;

		/// 用户手册按钮动作
		QAction*		m_pActionHelp;

		/// 在线更新动作
		QAction*		m_pActionUpdate;

		///官网按钮
		QAction*		m_pOfficeWebsite;
	};
}
#endif // VIEW_HELP_MENU_H
