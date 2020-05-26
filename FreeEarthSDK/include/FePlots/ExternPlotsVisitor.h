/**************************************************************************************************
* @file CExternPlotsVisitor.h
* @note 扩展军标的遍历器基类
* @author w00040
* @data 2017-11-2
**************************************************************************************************/
#ifndef EXTERN_PLOTS_VISITOR_H
#define EXTERN_PLOTS_VISITOR_H

#include <FePlots/Export.h>

#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	class FeExtNode::CExternNode;
	class FeExtNode::CExComposeNode;
	class CLinePlot;
	class CPolygonPlot;
	class CStraightArrow;
	class CStraightMoreArrow;
	class CDovetailDiagonalArrow;
	class CDovetailDiagonalMoreArrow;
	class CDoveTailStraightArrow;
	class CDoveTailStraightMoreArrow;
	class CCloseCurve;
	class CDiagonalArrow;
	class CDiagonalMoreArrow;
	class CDoubleArrow;
	class CGatheringPlace;
	class CCloseCurve;
	class CRoundedRect;
	//class CCurveFlag;
	//class CRectFlag;
	//class CTriangleFlag;
	class CBezierCurveArrow;
	class CPolyLineArrow;
	class CParallelSearch;
	class CCardinalCurveArrow;
	class CSectorSearch;
	class CStraightLineArrow;
	//class CFreeLine;
	//class CFreePolygon;
	/**
	* @class CExternPlotsVisitor
	* @note 扩展军标的遍历器基类，用户可以通过继承该类实现自己的遍历器
	* @author w00040
	*/
	class FEPLOTS_EXPORT CExternPlotsVisitor : public FeExtNode::CExternNodeVisitor
	{
	public:
		/**  
		  * @note 构造函数
		*/
		CExternPlotsVisitor();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExternPlotsVisitor();

	public:
		/**  
		  * @brief 根据类型进入具体类型访问器添加不同的标签页
		*/
		virtual bool VisitEnter(FeExtNode::CExternNode& externNode){return true;}

		virtual bool VisitEnter(FeExtNode::CExComposeNode& externNode) {return true;}

		virtual bool VisitExit(FeExtNode::CExComposeNode& externNode) {return true;}

		virtual bool VisitEnter(CLinePlot& externNode){ return true; }

		virtual bool VisitEnter(CPolygonPlot& externNode){ return true; }

		virtual bool VisitEnter(CStraightArrow& externNode){ return true; }

		virtual bool VisitEnter(CStraightMoreArrow& externNode){ return true; }

		virtual bool VisitEnter(CDovetailDiagonalArrow& externNode){ return true; }

		virtual bool VisitEnter(CDovetailDiagonalMoreArrow& externNode){ return true; }

		virtual bool VisitEnter(CDoveTailStraightArrow& externNode){ return true; }

		virtual bool VisitEnter(CDoveTailStraightMoreArrow& externNode){ return true; }

		virtual bool VisitEnter(CDiagonalArrow& externNode){ return true; }

		virtual bool VisitEnter(CDiagonalMoreArrow& externNode){ return true; }

		virtual bool VisitEnter(CDoubleArrow& externNode){ return true; }

		virtual bool VisitEnter(CCloseCurve& externNode){ return true; }

		virtual bool VisitEnter(CGatheringPlace& externNode){ return true; }

		virtual bool VisitEnter(CRoundedRect& externNode){ return true; }

		//virtual bool VisitEnter(CCurveFlag& externNode){ return true; }
		//
		//virtual bool VisitEnter(CRectFlag& externNode){ return true; }
		//
		//virtual bool VisitEnter(CTriangleFlag& externNode){ return true; }

		virtual bool VisitEnter(CBezierCurveArrow& externNode){ return true; }

		virtual bool VisitEnter(CPolyLineArrow& externNode){ return true; }

		virtual bool VisitEnter(CParallelSearch& externNode){ return true; }

		virtual bool VisitEnter(CCardinalCurveArrow& externNode){ return true; }

		virtual bool VisitEnter(CSectorSearch& externNode){ return true; }

		virtual bool VisitEnter(CStraightLineArrow& externNode){ return true; }

		//virtual bool VisitEnter(CFreeLine& externNode){ return true; }
		//
		//virtual bool VisitEnter(CFreePolygon& externNode){ return true; }
	};

}
#endif
