/**************************************************************************************************
* @file PlotPropertyBuilder.h
* @note 军标属性框
* @author w00040
* @data 2017-11-14
**************************************************************************************************/
#ifndef PLOT_PROPERTY_BUILDER_H
#define PLOT_PROPERTY_BUILDER_H

#include <QWidget>

#include <FePlots/ExternPlotsVisitor.h>
#include <FeShell/SystemService.h>

namespace FreeViewer
{
	class CPropertyWidget;

	/**
	  * @class CPlotPropertyBuilder
	  * @brief 军标属性框
	  * @note 创建军标属性对话框
	  * @author w00040
	*/
	class CPlotPropertyBuilder : public FePlots::CExternPlotsVisitor
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CPlotPropertyBuilder(FeShell::CSystemService* pSystemService, 
			FeExtNode::CExternNode* pMark,
			bool bCreate = false);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CPlotPropertyBuilder(void);

		/**  
		  * @brief 创建属性对话框
		*/
		QWidget* CreatePropertyWidget(QWidget* pParent);

		/**  
		  * @brief 根据类型进入具体类型访问器添加不同的标签页
		*/
		virtual bool VisitEnter(FeExtNode::CExComposeNode& externNode);
		virtual bool VisitEnter(FePlots::CStraightArrow& externNode);
		virtual bool VisitEnter(FePlots::CStraightMoreArrow& externNode);
		virtual bool VisitEnter(FePlots::CDovetailDiagonalArrow& externNode);
		virtual bool VisitEnter(FePlots::CDovetailDiagonalMoreArrow& externNode);
		virtual bool VisitEnter(FePlots::CDoveTailStraightArrow& externNode);
		virtual bool VisitEnter(FePlots::CDoveTailStraightMoreArrow& externNode);
		virtual bool VisitEnter(FePlots::CDiagonalArrow& externNode);
		virtual bool VisitEnter(FePlots::CDiagonalMoreArrow& externNode);
		virtual bool VisitEnter(FePlots::CDoubleArrow& externNode);
		virtual bool VisitEnter(FePlots::CCloseCurve& externNode);
		virtual bool VisitEnter(FePlots::CGatheringPlace& externNode);
		//virtual bool VisitEnter(FePlots::CCurveFlag& externNode);
		//virtual bool VisitEnter(FePlots::CRectFlag& externNode);
		//virtual bool VisitEnter(FePlots::CTriangleFlag& externNode);
		virtual bool VisitEnter(FePlots::CRoundedRect& externNode);
		virtual bool VisitEnter(FePlots::CBezierCurveArrow& externNode);
		virtual bool VisitEnter(FePlots::CPolyLineArrow& externNode);
		virtual bool VisitEnter(FePlots::CSectorSearch& externNode);
		virtual bool VisitEnter(FePlots::CParallelSearch& externNode);
		virtual bool VisitEnter(FePlots::CCardinalCurveArrow& externNode);
		virtual bool VisitEnter(FePlots::CStraightLineArrow& externNode);
		//virtual bool VisitEnter(FePlots::CFreeLine& externNode);
		//virtual bool VisitEnter(FePlots::CFreePolygon& externNode);

	protected:
		/**  
		  * @brief 创建属性框通用控件
		*/
		bool BuildCommon();

	public:
		bool				                          m_bCreate; 
		/// 属性对话框窗口
		CPropertyWidget*	                          m_pPropertyWidget;

		/// 关联的标记对象
		osg::observer_ptr<FeExtNode::CExternNode>     m_opExternNode;

		/// 系统服务对象
		osg::observer_ptr<FeShell::CSystemService>    m_opSystemService;
	};
}

#endif