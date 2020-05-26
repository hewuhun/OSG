/**************************************************************************************************
* @file FreeMainWindow.h
* @note 主窗口，包含二三维显示窗口、菜单栏、状态栏以及左侧树形列表框
* @author c00005
* @data 2017-1-3
**************************************************************************************************/
#ifndef FREE_MAIN_WINDOW_H
#define FREE_MAIN_WINDOW_H 1

#include <mainWindow/FreeFramelessWidget.h>
#include <mainWindow/BootScreenFrame.h>
#include <mainWindow/UIObserver.h>
#include <mainWindow/SimicircleMenu.h>
#include <mark/MarkPluginInterface.h>
#include <plot/PlotPluginInterface.h>
#include <measure/MeasurePluginInterface.h>
#include <mainWindow/FreeServiceDockTree.h>
#include <FeEarth/3DSceneWidget.h>
#include <service/ServicePluginInterface.h>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QHideEvent>
#include <QShowEvent>

namespace FreeViewer
{
	class CFreeSceneWidget;
	class CFreeMenuWidget;
	class CFreeStatusBar;
	class CDockFrame;
	class CFree3DDockTreeWidget;
	class CBootScreenFrame;
	class CUIObserver;
	class CFreeMapWidget;

	/**
	  * @class CFreeMainWindow
	  * @brief 主窗口类
	  * @note 主窗口，包含二三维显示窗口、菜单栏、状态栏以及左侧树形列表框
	  * @author c00005
	*/
	class CFreeMainWindow : public CFreeFramelessWidget
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        CFreeMainWindow(QWidget *parent = 0);
		
		/**  
		  * @note 析构函数
		*/
		~CFreeMainWindow();

	public:
		/**  
		  * @note 获取菜单栏
		*/
		CFreeMenuWidget* GetMenuWidget();

		/**  
		  * @note 获取状态条 
		*/
		CFreeStatusBar* GetStatusBar();
		
		/**  
		  * @note 获取左侧树列表
		*/
		CDockFrame*	GetDockWidget();

		/**  
		  * @note 获取三维渲染窗口 
		*/
		FeEarth::C3DSceneWidget* Get3DSceneWidget();
		
		/**  
		  * @note 获取三维场景树控件 
		*/
		CFree3DDockTreeWidget* Get3DTreeWidget();


				/**  
		  * @brief 获取服务管理树
		*/
		CFreeServiceDockTree* GetServiceDockTree();


		/**  
		  * @brief 清除标记
		*/
		void ClearMark();

		/**  
		  * @brief 清除军标
		*/
		void ClearPlot();

		/**  
		  * @brief 清除测量
		*/
		void ClearMeasure();

	protected:
		/**  
		  * @note 初始化窗口 
		*/
		void InitWidget();

		/**  
		  * @note 初始化窗口上下文
		*/
		void InitContext();

		/**  
		  * @brief 窗体移动
		*/
		void MoveWindows();

	protected:
		/**  
		  * @brief 显示事件
		*/
        virtual void showEvent(QShowEvent *event);

		/**  
		  * @brief 隐藏事件
		*/
		virtual void hideEvent(QHideEvent *event);

		/**  
		  * @note 重写绘制事件，用于实时绘制边框线
		*/
		virtual void paintEvent(QPaintEvent * event);

		/**  
		  * @brief 窗体大小改变事件
		*/
        virtual void resizeEvent(QResizeEvent * event);

		/**  
		  * @brief 窗体移动事件
		*/
		virtual void moveEvent(QMoveEvent *event);

		/**  
		  * @brief 事件过滤，用于鼠标移动到边缘判断，用于拖动缩放窗口
		*/
		virtual bool eventFilter(QObject* watched, QEvent* event);

	public slots:
		void Slot3DInitDone(bool);
		/**  
		  * @brief 拖拽进入事件槽函数
		*/
		void SlotDragEnterEvent(QDragEnterEvent *event);

		/**  
		  * @brief 放置事件槽函数
		*/
		void SlotDropEvent(QDropEvent *event);

		/**  
		  * @brief 树节点拖拽槽函数
		*/
		void SlotDragItem(bool bStatus);
	signals:
		/**  
		  * @brief 发送放置服务图层到主窗体信号
		*/
		void SiganlAddLayerItem(QTreeWidgetItem *pItem);
	protected:
		///菜单栏
		CFreeMenuWidget*					m_pFreeMenuWidget;

		///状态栏
		CFreeStatusBar*						m_pStatusBar;

		///三维渲染窗口
		FeEarth::C3DSceneWidget*			m_p3DSceneWidget;

		///二维渲染窗口
		CFreeMapWidget *                       m_p2DSceneWidget;

		///左侧树形框
		CDockFrame*							m_pDockWidget;

		typedef std::vector<CUIObserver*>	PluginList;
		PluginList							m_listPlugins;

	private:
		///启动界面
		CBootScreenFrame*					m_pBootScreenFrame; 

		//二三维显示窗体
		CFreeSceneWidget*					m_pSceneWidget;

		///三维场景树形框
		CFree3DDockTreeWidget*				m_p3DTreeWidget;

		// 服务管理树
		CFreeServiceDockTree*				m_pServiceDockTree;

		///测量菜单
		CMeasurePluginInterface*			m_pMeasureWidget;

		///标记菜单
		CMarkPluginInterface*				m_pMarkWidget;

		// 军标菜单
		CPlotPluginInterface*				m_pPlotWidget;

		// 服务管理接口
		CServicePluginInterface*			m_pServicePluginInterface;

		QGraphicsScene*						m_pGraphicScene;
		QGraphicsView*						m_pGraphicView;
		// 树节点拖拽离开标识
		bool								m_bDragLeaveFlag;
	};
}

#endif 
