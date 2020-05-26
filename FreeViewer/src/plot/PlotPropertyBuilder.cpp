#include <plot/PlotPropertyBuilder.h>
#include <mark/PropertyWidget.h>
#include <mark/ViewPointStyleWidget.h>
#include <mark/DescribeStyleWidget.h>
#include <mark/LineStyleWidget.h>

#include <FeUtils/StrUtil.h>
#include <FeExtNode/ExComposeNode.h>
#include <FePlots/StraightArrow.h>
#include <FePlots/DovetailDiagonalArrow.h>
#include <FePlots/DovetailDiagonalMoreArrow.h>
#include <FePlots/DoveTailStraightArrow.h>
#include <FePlots/DoveTailStraightMoreArrow.h>
#include <FePlots/DiagonalArrow.h>
#include <FePlots/DiagonalMoreArrow.h>
#include <FePlots/DoubleArrow.h>
#include <FePlots/CloseCurve.h>
//#include <FePlots/RectFlag.h>
//#include <FePlots/CurveFlag.h>
//#include <FePlots/TriangleFlag.h>
#include <FePlots/RoundedRect.h>
#include <FePlots/GatheringPlace.h>
#include <FePlots/BezierCurveArrow.h>
#include <FePlots/PolylineArrow.h>
#include <FePlots/ParallelSearch.h>
#include <FePlots/CardinalCurveArrow.h>
#include <FePlots/SectorSearch.h>
#include <FePlots/StraightMoreArrow.h>
#include <FePlots/StraightLineArrow.h>
//#include <FePlots/Freeline.h>
//#include <FePlots/FreePolygon.h>
namespace FreeViewer
{
	CPlotPropertyBuilder::CPlotPropertyBuilder(
		FeShell::CSystemService* pSystemService, 
		FeExtNode::CExternNode* pMark,
		bool bCreate)
		: FePlots::CExternPlotsVisitor()
		, m_bCreate(bCreate)
		, m_opSystemService(pSystemService)
		, m_pPropertyWidget(NULL)
		, m_opExternNode(pMark)
	{

	}

	CPlotPropertyBuilder::~CPlotPropertyBuilder(void)
	{
	}

	QWidget* CPlotPropertyBuilder::CreatePropertyWidget( QWidget* pParent )
	{
		if(m_opExternNode.valid())
		{
			m_pPropertyWidget = new CPropertyWidget(m_opExternNode.get(), pParent, m_bCreate);

			if(BuildCommon()) 
			{
				m_opExternNode->Accept(*this);
			}
		}

		return m_pPropertyWidget;
	}

	bool CPlotPropertyBuilder::BuildCommon()
	{
		bool bComposeNode = true;			//是否为文件夹
		if(m_opExternNode.valid())
		{
			/// 文件夹不添加视点信息
			if(!dynamic_cast<FeExtNode::CExComposeNode*>(m_opExternNode.get()))
			{
				CViewPointStyleWidget* pViewPointTab = new CViewPointStyleWidget(m_opSystemService.get(),
					m_opExternNode.get(),m_bCreate,m_pPropertyWidget);
				m_pPropertyWidget->AddTab(pViewPointTab);
			}
			else
			{
				bComposeNode = false;
				m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_COMPOSE_NODE);
			}

			CDescribeStyleWidget* pDescribeTab = new CDescribeStyleWidget(m_opExternNode.get(),m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pDescribeTab);

			return bComposeNode;
		}

		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FeExtNode::CExComposeNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_COMPOSE_NODE);
			return true;
		}

		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CStraightArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_STRAIGHT_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CDovetailDiagonalArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_DOVETAIL_DIAGONAL_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CDovetailDiagonalMoreArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_DOVETAIL_DIAGONAL_MORE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CDoveTailStraightArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_DOVETAIL_STRAIGHT_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CDoveTailStraightMoreArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_DOVETAIL_STRAIGHT_MORE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CDiagonalArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_DIAGONAL_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CDiagonalMoreArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_DIAGONAL_MORE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CDoubleArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_DOUBLE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CCloseCurve& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_CLOSE_CURVE);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CRoundedRect& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_ROUNDED_RECT);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	/*
	bool CPlotPropertyBuilder::VisitEnter( FePlots::CRectFlag& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_RECT_FLAG);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CCurveFlag& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_CURVE_FLAG);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CTriangleFlag& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_TRIANGLE_FLAG);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}
	*/

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CGatheringPlace& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_GATHERING_PLACE);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CBezierCurveArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_BEZIER_CURVE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CPolyLineArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_POLY_LINE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CSectorSearch& externNode )
	{

		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_SECTOR_SEARCH);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CParallelSearch& externNode )
	{

		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_PARALLEL_SEARCH);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CCardinalCurveArrow& externNode )
	{

		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_CARDINAL_CURVE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CStraightMoreArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_STRAIGHT_MORE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CStraightLineArrow& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_STRAIGHT_LINE_ARROW);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	/*
	bool CPlotPropertyBuilder::VisitEnter( FePlots::CFreeLine& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_FREE_LINE);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CPlotPropertyBuilder::VisitEnter( FePlots::CFreePolygon& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_FREE_POLYGON);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}
	*/

}