/**************************************************************************************************
* @file OGCServiceWidget.h
* @note OGC标准服务窗体
* @author w00040
* @data 2018-7-23
**************************************************************************************************/
#ifndef OGC_SERVICE_WIDGET_H
#define OGC_SERVICE_WIDGET_H 1

#include <mainWindow/FreeDialog.h>
#include <mainWindow/FreeServiceDockTree.h>

#include <FeServiceProvider/ServiceProvider.h>
#include <FeServiceProvider/WMSServiceProvider.h>
#include <FeServiceProvider/WMTSServiceProvider.h>
#include <FeServiceProvider/WFSServiceProvider.h>
#include <FeServiceProvider/WCSServiceProvider.h>

#include "ui_OGCServiceWidget.h"

namespace FreeViewer
{
	enum EOGCType
	{
		E_WMS = 0,
		E_WMTS,
		E_WFS,
		E_WCS
	};

	/**
	  * @class COGCServiceWidget
	  * @brief OGC标准服务窗体
	  * @note 添加WMS、WMTS、WFS、WCS、TMS服务
	  * @author w00040
	*/
	class COGCServiceWidget : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        COGCServiceWidget(QWidget *parent = 0);
		
		/**  
		  * @note 析构函数
		*/
		~COGCServiceWidget();
		
		/**  
		  * @note 初始化窗口 
		*/
		void InitWidget();

		/**  
		  * @brief 重置控件
		*/
		void Reset(QTreeWidgetItem *item);
		void Reset(EServiceItemType itemType);

		/**  
		  * @brief 查看服务，服务连接成功，显示图层列表
		*/
		void ShowLayerList(FeServiceProvider::ServiceProvider *pServiceProvider);

		/**  
		  * @brief 查看服务，服务连接失败，显示错误连接
		*/
		void ShowLinkError();

		/**  
		  * @brief 添加WMS
		*/
		void AddWMS(QString strName, QString strUrl, QString strVersion, QString usrName, QString passWord);

		/**  
		  * @brief 添加WMTS
		*/
		void AddWMTS(QString strName, QString strUrl, QString strVersion, QString usrName, QString passWord);

		/**  
		  * @brief 添加WFS
		*/
		void AddWFS(QString strName, QString strUrl, QString strVersion, QString usrName, QString passWord);

		/**  
		  * @brief 添加WCS
		*/
		void AddWCS(QString strName, QString strUrl, QString strVersion, QString usrName, QString passWord);

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
		 * @brief 关闭按钮槽函数
		 */
		virtual void SlotCloseBtnClicked();

		/**  
		  * @brief 获取图层按钮槽函数
		*/
		void SlotGetLayerBtnClicked();

		/**  
		  * @brief 类型改变槽函数
		*/
		void SlotTypeChanged(int nIndex);

		/**  
		  * @note 限制输入空格
		*/
		void SlotEditTextChanged();

		/**  
		  * @note 地址栏点击事件
		*/
		void SlotUrlClicked();

	signals:
		/**  
		  * @brief 发送添加OGC服务信号
		*/
		void SignalAddOGCService(FeServiceProvider::ServiceProvider *pServiceProvider);

		/**  
		  * @brief 服务添加完成信号
		*/
		void SignalAddServiceFinished();

		/**  
		  * @brief 重置当前激活节点为空信号
		*/
		void SignalResetCurrentItem();

	private:
		Ui::OGCServiceWidget				ui;

		//当前地址是否是示例地址 示例地址无效
		bool												m_bExampleUrl;
	};
}

#endif 
