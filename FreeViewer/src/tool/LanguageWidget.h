/**************************************************************************************************
* @file LanguageWidget.h
* @note 语言设置
* @author w00040
* @data 2017-2-28
**************************************************************************************************/
#ifndef LANGUAGE_SET_WIDGET_H
#define LANGUAGE_SET_WIDGET_H 1

#include <mainWindow/FreeDialog.h>

#include <FeShell/LanSerializer.h>
#include <FeUtils/PathRegistry.h>

#include "ui_LanguageWidget.h"

namespace FreeViewer
{
    /**
      * @class CLanguageWidget
      * @brief 语言设置
      * @note 中英文语言切换
      * @author w00040
    */
    class CLanguageWidget : public CFreeDialog
    {
        Q_OBJECT

    public:
		/**  
		  * @brief 构造函数
		*/
        CLanguageWidget(QWidget* parent = 0);

		/**  
		  * @brief 析构函数
		*/
        ~CLanguageWidget();

		/**  
		  * @brief 显示语言设置对话框
		*/
		void show();

    protected:
		/**  
		  * @brief 初始化界面
		*/
		void InitWidget();

		/**  
		  * @brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

		void ApplyLanguageChange(const FeShell::CLanConfig& config);
        
	public slots:
		/**  
		  * @brief 语言改变槽函数
		*/
		void SlotLanguageChanged(QString);

		/**  
		  * @brief 确认按钮槽函数
		*/
		void SlotOK();

		/**  
		  * @brief 取消按钮槽函数
		*/
		void SlotCancel();

    private:
        Ui::language_widget    ui;
		FeShell::CLanConfig		m_lanConfig;
		FeShell::CLanConfig		m_preLanConfig;

		///默认语言
		QString					m_strDefLan;
    };

}
#endif // LANGUAGE_SET_WIDGET_H
