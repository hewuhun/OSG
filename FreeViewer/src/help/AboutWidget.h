/**************************************************************************************************
* @file AboutWidget.h
* @note 关于界面
* @author x00028
* @data 2018-9-4
**************************************************************************************************/

#ifndef	ABOUT_WIDGTH_H
#define ABOUT_WIDGTH_H 1

#include <mainWindow/FreeDialog.h>
#include <ui_AboutWidget.h>

namespace FreeViewer 
{
	class CAboutWidget :  public CFreeDialog
	{
		Q_OBJECT

	public:
		CAboutWidget(QWidget*parent = 0);

		~CAboutWidget();

	private:
		Ui::AboutWidget							ui;
	};

}

#endif //ABOUT_WIDGTH_H
