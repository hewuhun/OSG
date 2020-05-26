/**************************************************************************************************
* @file OGCServicePropertyWidget.h
* @note 服务属性对话框
* @author w00040
* @data 2018-7-26
**************************************************************************************************/
#ifndef OGC_SERVICE_PROPERTY_WIDGET_H
#define OGC_SERVICE_PROPERTY_WIDGET_H 1

#include <mainWindow/FreeDialog.h>

#include <FeServiceProvider/ServiceProvider.h>

#include <QTreeWidgetItem>

#include "ui_OGCServicePropertyWidget.h"

namespace FreeViewer
{
	/**
	  * @class COGCServicePropertyWidget
	  * @brief OGC服务属性对话框
	  * @note 显示OGC服务属性
	  * @author w00040
	*/
	class COGCServicePropertyWidget : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        COGCServicePropertyWidget(QWidget *parent = 0);
		
		/**  
		  * @note 析构函数
		*/
		~COGCServicePropertyWidget();
		
		/**  
		  * @note 初始化窗口 
		*/
		void InitWidget();

		/**  
		  * @brief 重置控件
		*/
		void Reset();

		/**  
		  * @brief 设置当前激活的树节点
		*/
		void SetItemCurrent(QTreeWidgetItem *pItem);

		/**  
		  * @brief 检查输入信息
		*/
		bool CheckInputInfo();

	protected slots:
		/**  
		  * @brief 图层列表点击槽函数
		*/
		void SlotLayerListClicked(QListWidgetItem *item);

		/**  
		  * @brief 确定按钮槽函数
		*/
		void SlotOkBtnClicked();

		/**  
		  * @brief 取消按钮槽函数
		*/
		void SlotCancleBtnClicked();

	private:
		Ui::OGCServicePropertyWidget			ui;

		// 当前激活的树节点
		QTreeWidgetItem*						m_pItemCurrent;

		// 服务
		FeServiceProvider::ServiceProvider*		m_pServiceProvider;
	};
}

#endif 
