/**************************************************************************************************
* @file FreeDockFrame.h
* @note 树型列表框布局
* @author w00040
* @data 2017-1-14
**************************************************************************************************/
#ifndef DOCK_FRAME_H
#define DOCK_FRAME_H

#include <QWidget>
#include <QVector>
#include <QPushButton>
#include <QVBoxLayout>

#include "ui_FreeDockFrame.h"
#include "FreeDockWidget.h"

namespace FreeViewer
{
	/**
	  * @class CDockFrame
	  * @brief 树型列表框布局
	  * @note 树型列表框布局，实现树型列表框的整体框架布局
	  * @author w00040
	*/
	class CDockFrame : public QWidget
	{
		Q_OBJECT

	public:
		/**  
		 * @brief 构造函数
		 */
		CDockFrame(QWidget *parent = 0);

		/**  
		 * @brief 析构函数
		 */
		~CDockFrame();

	public:
		/**  
		 * @brief 设置宽度
		 */
		void SetFixedWidth(int nTotalWidth, int nTitleWidth=30);

		/**
		 * @brief 初始化数据
		 */
		void InitData();

		/**
		 * @brief 创建TitleBar
		 */
		void CreateTitleBar();

		/**
		 * @brief 添加悬浮窗到布局
		 */
		void AddDockWidget(CFreeDockWidget *dockWidget);

		/**  
		  * @brief 添加标题栏按钮到布局
		*/
		void AddPushBtnToLayout(QPushButton *pushBtn);

		/**
		 * @brief 所有悬浮窗都隐藏时设置窗体大小
		 */
		void SetWindowSize();

		/**
		 * @brief 获取标题栏
		 */
		QWidget* GetTitleBar();

	protected slots:

		/**
		 * @brief 悬浮窗隐藏时接收标题栏文本
		 */
		void SlotReceiveTitleText(const QString str);

		/**
		 * @brief 点击的标题栏
		 */
		void SlotClickedTitleBar(QString text);


	public:
		Ui::CDockFrame ui;

	protected:
		///竖向标题栏
		QWidget*						m_pWidgetTitleBar;

		///竖向标题栏布局
		QVBoxLayout*					m_pVLayout;

		///标题栏按钮容器
		QVector<QPushButton*>			m_VecBtns;

		///标题栏文本
		QStringList						m_titleList;

		///悬浮窗
		QVector<CFreeDockWidget *>		m_vecDocks;

		///默认窗体宽度
		int								m_nWinWidth;

		///默认竖向标题栏宽度
		int								m_nTitleBarWidth;

		///悬浮窗总数
		int								m_nDockCount;

		///悬浮窗隐藏数
		int								m_nDockHideCount;
	};
}

#endif // DOCKFRAME_H
