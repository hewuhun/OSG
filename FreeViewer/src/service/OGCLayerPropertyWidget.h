/**************************************************************************************************
* @file OGCLayerPropertyWidget.h
* @note 服务图层属性对话框
* @author w00040
* @data 2018-7-26
**************************************************************************************************/
#ifndef OGC_LAYER_PROPERTY_WIDGET_H
#define OGC_LAYER_PROPERTY_WIDGET_H 1

#include <mainWindow/FreeDialog.h>

#include <FeServiceProvider/ServiceProvider.h>

#include <QTreeWidgetItem>

#include "ui_OGCLayerPropertyWidget.h"

namespace FreeViewer
{
	/**
	  * @class COGCLayerPropertyWidget
	  * @brief OGC图层属性对话框
	  * @note 显示OGC图层属性
	  * @author w00040
	*/
	class COGCLayerPropertyWidget : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        COGCLayerPropertyWidget(QWidget *parent = 0);
		
		/**  
		  * @note 析构函数
		*/
		~COGCLayerPropertyWidget();
		
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
		Ui::OGCLayerPropertyWidget					ui;

		// 当前激活的树节点
		QTreeWidgetItem*							m_pItemCurrent;

		// 服务
		FeServiceProvider::ServiceProvider*			m_pServiceProvider;

		// 服务图层
		FeServiceProvider::ServiceLayer*			m_pServiceLayer;
	};
}

#endif 
