#include <plot/PlotVisitProcessor.h>

#include <FeUtils/PathRegistry.h>
#include <FeExtNode/ExternNode.h>
#include <FePlots/LinePlot.h>
#include <FePlots/PolygonPlot.h>
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
#include<FePlots/StraightMoreArrow.h>
#include <FePlots/StraightLineArrow.h>
//#include <FePlots/Freeline.h>
//#include <FePlots/FreePolygon.h>
#include <plot/PlotPluginInterface.h>
#include <mainWindow/FreeUtil.h>

#ifdef __QT4_USE_UNICODE
#else
#pragma execution_character_set("UTF-8")
#endif

namespace FreeViewer
{
	CPlotVisitProcessor::CPlotVisitProcessor() 
		: m_eProcessStyle(E_PLOT_DO_NOTHING)
		, m_bPlotFlag(true)
	{
	}

	void CPlotVisitProcessor::CommonTypeHandle( FeExtNode::CExFeatureNode& externNode )
	{
		switch (m_eProcessStyle)
		{
		case E_PLOT_SET_POSITION:
			{
				osgEarth::GeoPoint* pGeoPoint = GetPlotData<osgEarth::GeoPoint>();
				if(pGeoPoint)
				{
					m_bPlotFlag = externNode.PushBackVertex(pGeoPoint->vec3d());
				}
			}break;

		case E_PLOT_INITIALIZE:
			{
				externNode.SetFillColor(osg::Vec4d(1.0, 0.0, 0.0, 0.8));
				externNode.SetLineColor(osg::Vec4d(1.0, 1.0, 0.0, 1.0));
				externNode.SetLineWidth(2.0);
			}break;
		}
	}

	void CPlotVisitProcessor::CommonTypeHandle( FePlots::CLinePlot& externNode )
	{
		CommonTypeHandle((FeExtNode::CExFeatureNode&)externNode);

		if(E_PLOT_UPDATE_POSITION == m_eProcessStyle)
		{
			osgEarth::GeoPoint* pGeoPoint = GetPlotData<osgEarth::GeoPoint>();
			if(pGeoPoint)
			{
				externNode.ReplaceBackVertex(pGeoPoint->vec3d());
			}
		}
	}

	void CPlotVisitProcessor::CommonTypeHandle( FePlots::CPolygonPlot& externNode )
	{
		CommonTypeHandle((FeExtNode::CExFeatureNode&)externNode);

		if(E_PLOT_UPDATE_POSITION == m_eProcessStyle)
		{
			osgEarth::GeoPoint* pGeoPoint = GetPlotData<osgEarth::GeoPoint>();
			if(pGeoPoint)
			{
				externNode.ReplaceBackVertex(pGeoPoint->vec3d());
			}
		}
	}

	bool CPlotVisitProcessor::VisitExit( FeExtNode::CExComposeNode& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/add_folder.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("文件夹")));
		}
		return true;
	}

	bool CPlotVisitProcessor::VisitEnter(FePlots::CStraightArrow& externNode)
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/StraightArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("直箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CStraightMoreArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/StraightMoreArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("多点直箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CDovetailDiagonalArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/DovetailDiagonalArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("燕尾斜箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CDovetailDiagonalMoreArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/DovetailDiagonalMoreArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("多点燕尾斜箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CDoveTailStraightArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/DoveTailStraightArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("燕尾直箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CDoveTailStraightMoreArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/DoveTailStraightMoreArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("多点燕尾直箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CDiagonalArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/DiagonalArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("斜箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CDiagonalMoreArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/DiagonalMoreArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("多点斜箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CDoubleArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/DoubleArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("双箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CRoundedRect& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/RoundedRect.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("圆角矩形")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}

		return true;
	}

	/*
	bool CPlotVisitProcessor::VisitEnter( FePlots::CTriangleFlag& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/attack.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("三角旗标")));
		}
		else if(E_PLOT_STOP_DRAW == m_eProcessStyle)
		{
			externNode.PushBackVertex(externNode.Back());
			
		}
		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CRectFlag& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/attack.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("直角旗标")));
		}
		else if(E_PLOT_STOP_DRAW == m_eProcessStyle)
		{
			externNode.PushBackVertex(externNode.Back());
		}
		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CCurveFlag& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/attack.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("曲线旗标")));
		}
		else if(E_PLOT_STOP_DRAW == m_eProcessStyle)
		{
			externNode.PushBackVertex(externNode.Back());
		}
		return true;
	}
	*/

	bool CPlotVisitProcessor::VisitEnter( FePlots::CGatheringPlace& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/GatheringPlace.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("聚集区")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CCloseCurve& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/CloseCurve.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("闭合曲线")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CBezierCurveArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/BezierCurveArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("贝塞尔曲线箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CPolyLineArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/PolyLineArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("折线箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CSectorSearch& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/SectorSearch.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("扇形搜索区")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CParallelSearch& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/ParallelSearch.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("平行搜索区")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CCardinalCurveArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/CardinalCurveArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("Cardinal曲线箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CStraightLineArrow& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/StraightLineArrow.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("直线箭头")));
		}
		else if(E_PLOT_LEFT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}
		else if(E_PLOT_RIGHT_STOP_DRAW == m_eProcessStyle)
		{
			m_bPlotFlag = externNode.PushBackVertex(externNode.Back());
		}

		return true;
	}

	/*
	bool CPlotVisitProcessor::VisitEnter( FePlots::CFreeLine& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/attack.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("自由线")));
		}
		else if(E_PLOT_STOP_DRAW == m_eProcessStyle)
		{
			externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}

	bool CPlotVisitProcessor::VisitEnter( FePlots::CFreePolygon& externNode )
	{
		CommonTypeHandle(externNode);

		if (E_PLOT_GET_ICON_PATH == m_eProcessStyle)
		{
			QString* strIcon = GetPlotData<QString>();
			if(strIcon) *strIcon = QString(":/images/icon/attack.png");
		}
		else if (E_PLOT_INITIALIZE == m_eProcessStyle)
		{
			externNode.SetName(ConvertToSDKEncoding(QString("手绘面")));
		}
		else if(E_PLOT_STOP_DRAW == m_eProcessStyle)
		{
			externNode.PushBackVertex(externNode.Back());
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}
		else if (E_PLOT_DOUBLE_STOP_DRAW == m_eProcessStyle)
		{
			externNode.DeleteVertex(externNode.GetNumVertex()-1);
		}

		return true;
	}
	*/

	void CPlotVisitProcessor::DoProcess( FeExtNode::CExternNode* pNode , E_Plot_Process_Type eProcessType)
	{
		if(pNode)
		{
			m_eProcessStyle = eProcessType;
			pNode->Accept(*this);
		}
	}

	CPlotVisitProcessor::~CPlotVisitProcessor()
	{

	}

	bool CPlotVisitProcessor::GetFlag()
	{
		return m_bPlotFlag;
	}

}


