/**************************************************************************************************
* @file SearchWidget.h
* @note 视点定位窗口
* @author w00040
* @data 2017-3-1
**************************************************************************************************/
#ifndef LOCATE_WIDGET_H
#define LOCATE_WIDGET_H 1

#include <QToolTip>
#include <QMouseEvent>

#include <mainWindow/FreeDialog.h>

#include <FeShell/SystemService.h>

#include "ui_LocateWidget.h"

namespace FreeViewer
{
    /**
      * @class CLocateWidget
      * @brief 视点定位窗口
      * @note 根据输入的坐标搜索定位视点
      * @author w00040
    */
    class CLocateWidget : public CFreeDialog
    {
        Q_OBJECT

    public:
		/**  
		  * @brief 构造函数
		*/
        CLocateWidget(QWidget* parent = 0);

		/**  
		  * @brief 析构函数
		*/
        ~CLocateWidget();

		/**  
		  * @brief 设置系统服务
		*/
		void SetSystemService(FeShell::CSystemService* pSystemService);

    protected:
		/**  
		  * @brief 初始化界面
		*/
		void InitWidget();

	protected slots:
		/**  
		  * @brief 搜索按钮槽函数
		*/
		void SlotLocateClicked();
		
	private:
		///搜索界面
        Ui::locate_widget								ui;

		///系统服务
		osg::observer_ptr<FeShell::CSystemService>      m_opSystemService;
    };

}
#endif //LOCATE_WIDGET_H