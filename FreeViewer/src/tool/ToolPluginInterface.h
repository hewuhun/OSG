/**************************************************************************************************
* @file ToolPluginInterface.h
* @note 工具的Widget界面
* @author w00040
* @data 2017-2-27
**************************************************************************************************/
#ifndef TOOL_PLUGIN_INTERFACE_H
#define TOOL_PLUGIN_INTERFACE_H 1

#include <mainWindow/UIObserver.h>

#include <tool/LanguageWidget.h>
#include <tool/BlackBoard.h>
#include <tool/BlackBoardControlBtns.h>
#include <tool/DropScreen.h>

namespace FreeViewer
{
    /**
      * @class CToolPluginInterface
      * @note 全局的工具菜单，集成了全局的工具设置
      * @author w00040
    */
	class CToolPluginInterface : public CUIObserver
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CToolPluginInterface(CFreeMainWindow* pMainWindow);

		/**  
		  * @brief 析构函数
		*/
		~CToolPluginInterface();

	protected:
		/**  
		  * @brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

	private slots:
		/**  
		  * @brief 语言框选择
		*/
		void SlotOpenOptionsSet();

		/**  
		  * @brief 黑板
		*/
		void SlotBlackBoard(bool);

		/**  
		  * @brief 截屏功能
		*/
		void SlotDropScreen();

	private:
		///语言设置对话框
		CLanguageWidget*				m_pLanguageWidget;

		///黑板
		CBlackBoard*					m_pBlackBoard;

		///黑板控制按钮
		CBlackBoardControlBtns*			m_pControlBtns;

		///截屏
		CDropScreen*					m_pDropScreen;
	};  

}
#endif // TOOL_PLUGIN_INTERFACE_H
