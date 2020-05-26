/**************************************************************************************************
* @file TileServiceWidget.h
* @note Tile服务窗体
* @author w00040
* @data 2018-7-23
**************************************************************************************************/
#ifndef TILE_SERVICE_WIDGET_H
#define TILE_SERVICE_WIDGET_H 1

#include <mainWindow/FreeDialog.h>
#include <mainWindow/FreeServiceDockTree.h>

#include <FeServiceProvider/ServiceProvider.h>
#include <FeServiceProvider/TMSServiceProvider.h>
#include <FeServiceProvider/XYZServiceProvider.h>

#include "ui_TileServiceWidget.h"

namespace FreeViewer
{
	enum ETileType
	{
		E_TMS = 0,
		E_XYZ
	};

	/**
	  * @class CTileServiceWidget
	  * @brief Tile服务窗体
	  * @note 添加Tile服务
	  * @author w00040
	*/
	class CTileServiceWidget : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        CTileServiceWidget(QWidget *parent = 0);
		
		/**  
		  * @note 析构函数
		*/
		~CTileServiceWidget();
		
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
		  * @brief 添加TMS
		*/
		void AddTMS(QString strName, QString strUrl);

		/**  
		  * @brief 添加XYZ
		*/
		void AddXYZ(QString strName, QString strUrl);

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
		  * @note 限制输入空格
		*/
		void SlotEditTextChanged();

		/**  
		  * @note 切换不同的驱动 显示不同的示例地址
		*/
		void SlotTypeChanged(QString);

		/**  
		  * @note 地址栏点击事件
		*/
		void SlotUrlClicked();

	signals:
		/**  
		  * @brief 发送添加Tile服务信号
		*/
		void SignalAddTileService(FeServiceProvider::ServiceProvider *pServiceProvider);

		/**  
		  * @brief 服务添加完成信号
		*/
		void SignalAddServiceFinished();

		/**  
		  * @brief 重置当前激活节点为空信号
		*/
		void SignalResetCurrentItem();

	private:
		Ui::TileServiceWidget				ui;

		//当前地址是否是示例地址 示例地址无效
		bool												m_bExampleUrl;
	};
}

#endif 
