/**************************************************************************************************
* @file UIObserver.h
* @note 主窗口观察类，其中包含了菜单条、工具条、渲染窗口等的引用
*			弹出对话框可从该类集成，实现添加菜单、工具等
* @author c00005
* @data 2017-1-12
**************************************************************************************************/
#ifndef UI_OBSERVER_H
#define UI_OBSERVER_H 1

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QMainWindow>
#include <QTextCodec>

#include <FeShell/SystemService.h>

namespace FreeViewer
{
	class CFreeMainWindow;
	class CFreeMenuWidget;
	class CFreeStatusBar;
	class CFreeToolBar;
	class CFreeToolButton;
	class CFreeSceneWidget;
	class CDockFrame;

	/**
	* @class CUIObserver
	* @note 主窗口观察类，其中包含了菜单条、工具条、渲染窗口等的引用
	*			弹出对话框可从该类集成，实现添加菜单、工具等
	* @author c00005
	*/
	class CUIObserver : public QObject
	{
	public:
		/**  
		* @note 构造函数  
		* @param pSystemService [in] 三维渲染窗口的系统服务
		* @param pMainWindow [in] 主窗口
		* @return 返回值
		*/
		CUIObserver(CFreeMainWindow* pMainWindow);

		/**  
		* @note 析构函数
		*/
		~CUIObserver();

	public:
		/**
		*@note: 同时创建菜单栏按钮和工具栏按钮，并将二者状态同步，连接信号时需使用toggled(bool)信号
		* @param strName [in] QAction的名称
		* @param strNormalIcon [in] 正常状态时工具栏的图标
		* @param strHoverIcon [in] 鼠标划过时工具栏的图标
		* @param strPressIcon [in] 鼠标按下时工具栏的图标
		* @param strDisableIcon [in] 工具按钮失效时图标
		* @param bCheckable [in] 是否可被选中
		* @param bChecked [in] 当前的选中状态
		*/
		virtual QAction* CreateMenuAndToolAction(
			QString strName, 
			QString strNormalIcon = "", 
			QString strHoverIcon = "",
			QString strPressIcon = "",
			QString strDisableIcon = "",
			bool bCheckable = true,
			bool bChecked = false);

		/**
		*@note: 创建菜单栏的QAction
		* @param strName [in] QAction的名称
		* @param bCheckable [in] 是否可被选中
		* @param bChecked [in] 当前的选中状态
		*/
		virtual QAction* CreateMenuAction(
			QString strName,
			bool bCheckable = false,
			bool bChecked = false);

		/**
		*@note: 创建工具栏按钮
		* @param strName [in] 工具栏按钮的名称
		* @param strNormalIcon [in] 正常状态时工具栏的图标
		* @param strHoverIcon [in] 鼠标划过时工具栏的图标
		* @param strPressIcon [in] 鼠标按下时工具栏的图标
		* @param strDisableIcon [in] 工具按钮失效时图标
		* @param bCheckable [in] 是否可被选中
		* @param bChecked [in] 当前的选中状态
		*/
		virtual CFreeToolButton* CreateToolBtn(
			QString strName, 
			QString strNormalIcon = "", 
			QString strHoverIcon = "",
			QString strPressIcon = "",
			QString strDisableIcon = "",
			bool bCheckable = true,
			bool bChecked = false);
		
		/**
		*@note: 获取主菜单中的工具条
		*/
		virtual CFreeToolBar* GetToolBar();

		/**
		*@note: 获取主窗口中的状态栏
		*/
		virtual CFreeStatusBar* GetStatusBar();

		/**  
		* @note 获取左侧树列表
		*/
		virtual CDockFrame*	GetDockWidget();

		/**
		*@note: 获取到三维渲染服务
		*/
		virtual FeShell::CSystemService* GetSystemService();

	protected:
		///菜单栏的标题名称
		QString							m_strMenuTitle;

		///主窗口
		CFreeMainWindow*				m_pMainWindow;

		///三维渲染服务
		osg::observer_ptr<FeShell::CSystemService> m_opSystemService;
	};
}
#endif //UI_OBSERVER_H
