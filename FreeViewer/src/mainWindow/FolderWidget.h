/**************************************************************************************************
* @file FolderWidget.h
* @note 添加文件夹对话框
* @author c00005
* @data 2017-4-8
**************************************************************************************************/
#ifndef FOLDER_WIDGET_H
#define FOLDER_WIDGET_H 1

#include <mainWindow/FreeDialog.h>

#include "ui_FolderWidget.h"

namespace FreeViewer
{
	/**
	  * @class CFolderWidget
	  * @brief 添加文件夹对话框
	  * @author c00005
	*/
    class CFolderWidget : public CFreeDialog
    {
        Q_OBJECT

    public:
		/**  
		  * @note 构造和析构函数  
		*/
		CFolderWidget(QWidget* pParent=0);

        ~CFolderWidget();

	protected:
		/**  
		  * @note 控件初始化
		*/
		void InitWidget();

		/**  
		  * @note 内容初始化
		*/
		void BuildContext();

	protected slots:
		/**  
		  * @note 确定取消处理函数
		*/
		virtual void reject();

		virtual void accept();

		virtual void SlotTextChanged(QString text);
	
	signals:
		/**  
		  * @note 确定取消处理信息号
		*/
		void SignalFolderName(QString strName, bool bCreate);

    private:
		Ui::CFolderWidget							ui;
    };

}
#endif // FOLDER_WIDGET_H
