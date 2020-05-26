/**************************************************************************************************
* @file ViewPointLocalDialog.h
* @note 视点获取定位属性对话框
* @author w00040
* @data 2017-3-2
**************************************************************************************************/
#ifndef VIEW_POINT_DIALOG_H
#define VIEW_POINT_DIALOG_H

#include <FeShell/SystemService.h>
#include <FeUtils/FreeViewPoint.h>

#include <mainWindow/FreeDialog.h>

#include "ui_ViewPointDialog.h"

namespace FreeViewer
{
	class CViewPointDialog : public CFreeDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CViewPointDialog(FeShell::CSystemService* pSystemService, QWidget* parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CViewPointDialog(void);

	protected:
		/**  
		  * @brief 初始化右键对话框界面
		*/
		void InitWidget();

		/**  
		  * @brief 检测输入的信息
		*/
		bool CheckInputInfo();

		/**  
		  * @brief 创建视点图层
		*/
		void CreateViewPointLayer();

	signals:
		/**  
		  * @brief 添加视点的信号
		*/
		void SignalAddViewPoint(FeUtil::CFreeViewPoint);

	protected slots:
		/**  
		  * @brief 当前视点按钮槽函数
		*/
		void SlotCurrentViewButtonClicked();

		/**  
		  * @brief 确认按钮槽函数
		*/
		void SlotOkButtonClicked();

		/**  
		  * @brief 取消按钮槽函数
		*/
		void SlotCancelButtonClicked();

	private:
		Ui::ViewPointDialog_Widget						ui;

		///系统服务
		osg::observer_ptr<FeShell::CSystemService>      m_opSystemService;
	};
}

#endif // VIEW_POINT_DIALOG_H
