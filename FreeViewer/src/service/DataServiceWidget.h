/**************************************************************************************************
* @file DataServiceWidget.h
* @note 数据集服务窗体
* @author w00040
* @data 2018-7-31
**************************************************************************************************/
#ifndef DATA_SERVICE_WIDGET_H
#define DATA_SERVICE_WIDGET_H 1

#include <mainWindow/FreeDialog.h>

#include <FeServiceProvider/ServiceProvider.h>
#include <FeServiceProvider/DataServiceProvider.h>

#include "ui_DataServiceWidget.h"

namespace FreeViewer
{
	/**
	  * @class CDataServiceWidget
	  * @brief 数据集服务窗体
	  * @note 添加数据集
	  * @author w00040
	*/
	class CDataServiceWidget : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        CDataServiceWidget(QWidget *parent = 0);
		
		/**  
		  * @note 析构函数
		*/
		~CDataServiceWidget();
		
		/**  
		  * @note 初始化窗口 
		*/
		void InitWidget();

		/**  
		  * @brief 重置控件
		*/
		void Reset();

		/**  
		  * @brief 添加数据
		*/
		void AddData(QString strName, QString strUrl, QString strDriver);

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

		/**  
		  * @note 限制输入空格
		*/
		void SlotEditTextChanged();

		/**  
		  * @note 切换不同的驱动 显示不同的示例地址
		*/
		void SlotDriverChanged(QString);

		/**  
		  * @note 地址栏点击
		*/
		void SlotUrlClicked();

	signals:
		/**  
		  * @brief 发送添加数据集服务信号
		*/
		void SignalAddDataService(FeServiceProvider::ServiceProvider *pServiceProvider);

		/**  
		  * @brief 服务添加完成信号
		*/
		void SignalAddServiceFinished();

	private:
		Ui::DataServiceWidget				ui;

		//当前地址是否是示例地址 示例地址无效
		bool												m_bExampleUrl;
	};
}

#endif 
