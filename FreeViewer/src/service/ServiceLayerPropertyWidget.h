/**************************************************************************************************
* @file ServiceLayerPropertyWidget.h
* @note 服务图层属性对话框（Tile服务图层和数据集服务图层）
* @author w00040
* @data 2018-7-26
**************************************************************************************************/
#ifndef SERVICE_LAYER_PROPERTY_WIDGET_H
#define SERVICE_LAYER_PROPERTY_WIDGET_H 1

#include <mainWindow/FreeDialog.h>

#include <FeServiceProvider/ServiceProvider.h>

#include <QTreeWidgetItem>

#include "ui_ServiceLayerPropertyWidget.h"

namespace FreeViewer
{
	
	class CServiceLayerPropertyWidget : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        CServiceLayerPropertyWidget(QWidget *parent = 0);
		
		/**  
		  * @note 析构函数
		*/
		~CServiceLayerPropertyWidget();
		
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
		  * @brief 确定按钮槽函数
		*/
		void SlotOkBtnClicked();

		/**  
		  * @brief 取消按钮槽函数
		*/
		void SlotCancleBtnClicked();

	private:
		Ui::ServiceLayerPropertyWidget				ui;

		// 当前激活的树节点
		QTreeWidgetItem*							m_pItemCurrent;

		// 服务
		FeServiceProvider::ServiceProvider*			m_pServiceProvider;

		// 服务图层
		FeServiceProvider::ServiceLayer*			m_pServiceLayer;
	};
}

#endif 
