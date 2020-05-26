/**************************************************************************************************
* @file PlotMenuManager.h
* @note 军标菜单管理类
* @author w00040
* @data 2017-11-3
**************************************************************************************************/
#ifndef PLOT_MENU_MANAGER_H
#define PLOT_MENU_MANAGER_H 1

#include <QObject>
#include <QMenu>
#include <QAction>

#include <FeExtNode/ExternNode.h>

namespace FreeViewer
{
	class CPlotPluginInterface;

	/**
	  * @class CPlotMenuManager
	  * @brief 军标菜单管理
	  * @note 集中管理菜单相关逻辑和操作
	  * @author w00040
	*/
	class CPlotMenuManager : public QObject
	{
		Q_OBJECT

	public:
		CPlotMenuManager(CPlotPluginInterface* pPlotPlugin);
			
		/**  
		  * @brief 初始化菜单
		*/
		void InitMenuAndActions();

		/**  
		  * @brief 为指定图元创建右键菜单
		*/
		QMenu* CreatePopMenu(FeExtNode::CExternNode* pExternNode, QWidget* pMenuParent);

		/**  
		  * @brief 为图元根节点创建右键菜单
		*/
		QMenu* CreateRootPopMenu(FeExtNode::CExternNode* pExternNode, QWidget* pMenuParent);

		/**  
		  * @brief 重置菜单状态
		*/
		void ResetMenu() { SetMutex(NULL); }

		/**  
		  * @brief 菜单互斥操作
		*/
		void SetMutex( QAction* pCurrentAction );

	private:
		/**  
		  * @brief 创建添加图元菜单
		*/
		QMenu* CreateAddMenu(QWidget* pMenuParent);

		/**  
		  * @brief 对标记的激活进行一致处理
		*/
		void HandlePlotAction(osg::ref_ptr<FeExtNode::CExternNode> pNode, QAction* pAction, bool bAction);

	signals:
		/**  
		  * @brief 右键打开属性对话框信号
		*/
		void SignalOpenPropertyWidget();

		/**  
		  * @brief 右键删除军标信号
		*/
		void SignalDeletePlot();

		/**  
		  * @brief 右键清除军标信号
		*/
		void SignalClearPlot();

	private slots:
		/**  
		  * @brief 添加文件夹槽函数
		*/
		void SlotAddFolder(bool bAction);

		/**  
		  * @brief 添加直箭头槽函数
		*/
		void SlotAddStraightArrow(bool bAction);

		/**  
		  * @brief 添加多点直箭头槽函数
		*/
		void SlotAddStraightMoreArrow(bool bAction);

		/**  
		  * @brief 添加燕尾斜箭头槽函数
		*/
		void SlotAddDovetailDiagonalArrow(bool bAction);

		/**  
		  * @brief 添加燕尾多点斜箭头槽函数
		*/
		void SlotAddDovetailDiagonalMoreArrow(bool bAction);

		/**  
		  * @brief 添加燕尾直箭头槽函数
		*/
		void SlotAddDoveTailStraightArrow(bool bAction);

		/**  
		  * @brief 添加燕尾多点直箭头槽函数
		*/
		void SlotAddDoveTailStraightMoreArrow(bool bAction);

		/**  
		  * @brief 添加斜箭头槽函数
		*/
		void SlotAddDiagonalArrow(bool bAction);

		/**  
		  * @brief 添加多点斜箭头槽函数
		*/
		void SlotAddDiagonalMoreArrow(bool bAction);

		/**  
		  * @brief 添加双箭头槽函数
		*/
		void SlotAddDoubleArrow(bool bAction);

		/**  
		  * @brief 添加闭合曲线槽函数
		*/
		void SlotAddCloseCurve(bool bAction);

		/**  
		  * @brief 添加矩形曲线槽函数
		*/
		//void SlotAddRectFlag(bool bAction);

		/**  
		  * @brief 添加曲线旗标槽函数
		*/
		//void SlotAddCurveFlag(bool bAction);

		/**  
		  * @brief 添加三角旗标槽函数
		*/
		//void SlotAddTriangleFlag(bool bAction);

		/**  
		  * @brief 添加圆角矩形槽函数
		*/
		void SlotAddRoundedRect(bool bAction);

		/**  
		  * @brief 添加聚集区槽函数
		*/
		void SlotAddGatheringPlace(bool bAction);

		/**  
		  * @brief 添加Bezier曲线箭头槽函数
		*/
		void SlotAddBezierCurveArrow(bool bAction);

		/**  
		  * @brief 添加折线箭头槽函数
		*/
		void SlotAddPolyLineArrow(bool bAction);

		/**  
		  * @brief 添加Cardinal曲线箭头槽函数
		*/
		void SlotAddCardinalCurveArrow(bool bAction);
		
		/**  
		  * @brief 添加扇形搜索区槽函数
		*/
		void SlotAddSectorSearch(bool bAction);

		/**  
		  * @brief 添加平行搜索区槽函数
		*/
		void SlotAddParallelSearch(bool bAction);

		/**  
		  * @brief 添加直线箭头槽函数
		*/
		void SlotAddStraightLineArrow(bool bAction);

		/**  
		  * @brief 添加自由线槽函数
		*/
		//void SlotAddFreeLine(bool bAction);

		/**  
		  * @brief 添加手绘面槽函数
		*/
		//void SlotAddFreePolygon(bool bAction);

	private:
		// 军标对应的Action
		QAction*						m_pFolderAction;
		QAction*						m_pStraightArrowAction;
		QAction*						m_pStraightMoreArrowAction;
		QAction*						m_pDovetailDiagonalArrowAction;
		QAction*						m_pDovetailDiagonalMoreArrowAction;
		QAction*						m_pDoveTailStraightArrowAction;
		QAction*						m_pDoveTailStraightMoreArrowAction;
		QAction*						m_pDiagonalArrowAction;
		QAction*						m_pDiagonalMoreArrowAction;
		QAction*						m_pDoubleArrowAction;
		//QAction*						m_pRectFlagAction;
		//QAction*						m_pCurveFlagAction;
		//QAction*						m_pTriangleFlagAction;
		QAction*						m_pRoundedRectAction;
		QAction*						m_pGatheringPlaceAction;
		QAction*						m_pCloseCurveAction;
		QAction*						m_pBezierCurveArrowAction;
		QAction*						m_pPolyLineArrowAction;
		QAction*						m_pParallelSearchAction;
		QAction*						m_pSectorSearchAction;
		QAction*						m_pCardinalCurveArrowAction;
		QAction*						m_pStraightLineArrowAction;
		//QAction*						m_pFreeLineAction;
		//QAction*						m_pFreePolygonAction;

		// 当前活动的Action
		QAction*						m_pActiveAction;  

		// 右键菜单相关 Action
		QAction*						m_pOpenPropertyAction;    
		QAction*						m_pDeletePlotAction;
		QAction*						m_pClearPlotAction;

	private:
		CPlotPluginInterface*			m_pPlotPlugin;
	};
}

#endif
