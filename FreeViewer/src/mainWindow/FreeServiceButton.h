/**************************************************************************************************
* @file FreeServiceButton.h
* @note 服务管理停靠窗按钮
* @author w00040
* @data 2018-7-30
**************************************************************************************************/
#ifndef FREE_SERVICE_BUTTON_H
#define FREE_SERVICE_BUTTON_H 1

#include <QWidget>

#include "ui_FreeServiceButton.h"

namespace FreeViewer
{
	enum EServiceButtonType
	{
		E_SERVICE_UPDATE_BTN = 0,//更新按钮
		E_SERVICE_EXPAND_BTN,//展开按钮
		E_SERVICE_COLLAPSE_BTN,//合并按钮
		E_SERVICE_DELETE_BTN//删除按钮
	};

	/**
	  * @class CFreeServiceButton
	  * @brief 服务管理停靠窗按钮
	  * @note 实现服务管理停靠窗按钮
	  * @author w00040
	*/
	class CFreeServiceButton : public QWidget
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        CFreeServiceButton(QWidget *parent = 0);
		
		/**  
		  * @note 析构函数
		*/
		~CFreeServiceButton();
		
		/**  
		  * @note 初始化窗口 
		*/
		void InitWidget();

	protected slots:
		/**  
		  * @brief 更新按钮槽函数
		*/
		void SlotUpdateBtnClicked();

		/**  
		  * @brief 树节点展开按钮槽函数
		*/
		void SlotExpandBtnClicked();

		/**  
		  * @brief 树节点合并按钮槽函数
		*/
		void SlotCollapseBtnClicked();

		/**  
		  * @brief 删除按钮槽函数
		*/
		void SlotDeleteBtnClicked();

	signals:
		/**  
		  * @brief 更新按钮点击信号
		*/
		void SignalUpdateBtnClicked(EServiceButtonType);

		/**  
		  * @brief 树节点展开按钮点击信号
		*/
		void SignalExpandBtnClicked(EServiceButtonType);

		/**  
		  * @brief 树节点合并按钮点击信号
		*/
		void SignalCollapseBtnClicked(EServiceButtonType);

		/**  
		  * @brief 删除按钮点击信号
		*/
		void SignalDeleteBtnClicked(EServiceButtonType);

	private:
		Ui::FreeServiceButton				ui;
	};
}

#endif 
