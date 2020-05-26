/**************************************************************************************************
* @file PlotPluginInterface.h
* @note 军标管理界面类
* @author w00040
* @data 2017-11-3
**************************************************************************************************/
#ifndef PLOT_PLUGIN_INTERFACE_H
#define PLOT_PLUGIN_INTERFACE_H

#include <QObject>
#include <QVector>

#include <FeManager/FreeMarkSys.h>

#include <plot/PlotMenuManager.h>
#include <plot/PlotEventHandler.h>
#include <plot/PlotVisitProcessor.h>
#include <mainWindow/UIObserver.h>
#include <mainWindow/Free3DDockTreeWidget.h>

#include <plot/PlotEventListener.h>

namespace FreeViewer
{
    /**
      * @class CPlotPluginInterface
      * @brief 军标管理界面
      * @note 对场景中所有军标进行管理
      * @author w00040
    */
    class CPlotPluginInterface : public CUIObserver
    {
		Q_OBJECT
    public:
		/**  
		  * @brief 构造函数
		*/
		CPlotPluginInterface(CFreeMainWindow* pMainWindow);

		/**  
		  * @brief 析构函数
		*/
        ~CPlotPluginInterface();

		/**  
		  * @brief 获取主界面
		*/
		CFreeMainWindow* GetMainWindow();

	protected:
		/**
		  *brief 初始化控件
		*/
		void InitWidget();
		
		/**
		  *brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

	public:
		/**  
		  * @brief 开始绘制军标
		*/
		void StartDrawPlot();

		/**  
		  * @brief 军标绘制成功，结束军标绘制，显示属性框
		*/
		void SuccessToDraw();

		/**  
		  * @brief 军标绘制失败，结束军标绘制，不显示属性框
		*/
		void FailedToDraw();

		/**  
		  * @brief 显示指定军标的属性对话框
		*/
		void ShowPlotPropertyWidget(FeExtNode::CExternNode* pNode);

		/**  
		  * @brief 判断当前是否有军标正在绘制或正在编辑
		*/
		bool IsDrawingOrEditing();

		/**  
		  * @brief 获取军标遍历处理器
		*/
		CPlotVisitProcessor& GetPlotProcessor();

		/**  
		  * @brief 开始军标事件捕获
		*/
		void StartEventCapture();

		/**  
		  * @brief 停止军标事件捕获
		*/
		void StopEventCapture();

		/**  
		  * @brief 获取场景管理系统
		*/
		FeManager::CFreeMarkSys* GetMarkSys();

		/**  
		  * @brief 获取树形控件
		*/
		CFree3DDockTreeWidget* GetTreeDockWidget();

		/**  
		  * @brief 设置当前激活的军标节点
		*/
		void SetActivePlotNode(FeExtNode::CExternNode* pNode);

		/**  
		  * @brief 清除当前军标
		*/
		void DeleteCurrentPlot();

		/**
		  *brief 初始化军标树形结构控件
		*/
		void InitPlotTree();

		/**
		  *brief 初始化菜单
		*/
		void InitPlotMenu();

		/**
		  *brief 添加军标到树形控件
		*/
		void AddTreeItem(QTreeWidgetItem* pParentItem, FeExtNode::CExternNode* pNode);
	
		/**  
		  * @brief 通过树节点获取与之关联的军标节点
		*/
		FeExtNode::CExternNode* GetPlotByItem(QTreeWidgetItem*);
		
		/**  
		  * @brief 获取当前树形节点所属组节点
		*/
		QTreeWidgetItem* GetCurrentGroupItem();

		/**  
		  * @brief 重置所有状态，所有操作结束后调用
		*/
		void ResetAllState();

		/**  
		  * @brief 从标记系统中删除指定节点，包括其所有子节点
		*/
		void DeleteNodeFromMarkSys(FeExtNode::CExternNode* pNode, FeExtNode::CExternNode* pParentNode);

	public slots:
		/// 属性对话框用户选择结果槽函数
		void SlotPropertyOK(FeExtNode::CExternNode* pNode);
		void SlotPropertyCancel(FeExtNode::CExternNode* pNode);

		/// 监听器卸载槽函数
		void SlotRemoveEventListener(FeExtNode::CExternNode* pNode);

		/// 响应树形控件的相关槽函数
		void SlotContextMenuItem(QTreeWidgetItem*, QPoint);
		void SlotItemsVisibleChange(QTreeWidgetItem*, bool);
		void SlotItemDBClicked(QTreeWidgetItem*);

		/// 右键菜单相关槽函数
		void SlotMenuOpenPropertyWidget();
		void SlotMenuDeletePlot();
		void SlotMenuClearPlot();

		/// SDK事件响应
		void SlotNodeStateChanged(unsigned int);

	private:
		/// 场景标记管理系统
		osg::ref_ptr<FeManager::CFreeMarkSys> m_rpMarkSys;

		/// 客户端军标事件处理对象
		osg::ref_ptr<CPlotEventHandler>       m_rpPlotHandler;

		/// 当前军标节点
		osg::ref_ptr<FeExtNode::CExternNode>  m_rpCurPlotNode;

		/// 是否正在创建军标
		bool                                  m_bCreatingPlot;

		/// 是否正在显示军标属性框
		bool                                  m_bPropertyWidgetShow;

		/// 树形控件
		CFree3DDockTreeWidget*                m_p3DTreeWidget;

		///军标节点的根节点
		QTreeWidgetItem*				      m_pPlotTreeItemRoot;

		///当前军标节点中激活的节点
		QTreeWidgetItem*					  m_pActiveItem;

		/// 军标遍历处理器
		CPlotVisitProcessor                   m_plotProcessor;

		/// 军标菜单管理器
		CPlotMenuManager                      m_plotMenuManager;

		///当前的属性对话框
		QWidget*							  m_pCurrentWidget;

		///SDK事件监听器
		osg::ref_ptr<CPlotEventListener>      m_rpEventListener;
	};

}

#endif