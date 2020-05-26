/**************************************************************************************************
* @file PlotVisitProcessor.h
* @note 场景标记遍历处理器
* @author w00040
* @data 2017-11-3
**************************************************************************************************/
#ifndef PLOT_VISIT_PROCESSOR_H
#define PLOT_VISIT_PROCESSOR_H 1

#include <FePlots/ExternPlotsVisitor.h>

namespace FeExtNode
{
	class CExFeatureNode;
	class CExLocalizedNode;
}

namespace FreeViewer
{
	 /**
      * @class CPlotVisitProcessor
      * @note 军标遍历处理器
      * @author w00040
    */
	class CPlotVisitProcessor : public FePlots::CExternPlotsVisitor
	{
	public:
		/// 军标的处理状态宏，处理器根据当前状态进行相应处理
		enum E_Plot_Process_Type
		{
			E_PLOT_DO_NOTHING = 0,    // 什么都不做

			E_PLOT_GET_ICON_PATH,     // 获取军标对应的icon路径
			E_PLOT_INITIALIZE,        // 军标的初始化操作

			E_PLOT_SET_POSITION,      // 设置位置，处理鼠标左键点击
			E_PLOT_SET_SIZE,          // 设置尺寸位置
			E_PLOT_UPDATE_POSITION,   // 更新位置，处理绘制过程中鼠标移动
			E_PLOT_LEFT_STOP_DRAW,	  // 结束绘制，处理鼠标左键点击后立即右键结束绘制事件
			E_PLOT_RIGHT_STOP_DRAW,        // 结束绘制，处理鼠标移动后立即右键点击结束绘制事件
			E_PLOT_DOUBLE_STOP_DRAW	  // 结束绘制，处理鼠标左键双击结束绘制事件
		};

	public:
		/// 数据基类，主要用于辅助转换
		struct CPlotDataBase{virtual ~CPlotDataBase(){}};

		/// 数据模板类，便于处理不同的数据
		template <class T>
		struct CProcessData : public CPlotDataBase
		{
			CProcessData(T& data): m_pData(&data) {}
		
			/// 要处理的数据
			T*    m_pData;       
		};

	public:
		/**  
		  * @brief 构造函数
		*/
		CPlotVisitProcessor();

		~CPlotVisitProcessor();

		/**  
		  * @brief 获取军标是否绘制标识
		*/
		bool GetFlag();
		
		/**  
		  * @brief 执行指定操作
		*/
		void DoProcess(FeExtNode::CExternNode* pNode, E_Plot_Process_Type eProcessType);

		/**  
		  * @brief 执行指定操作，携带处理数据
		*/
		template <class T>
		void DoProcess(FeExtNode::CExternNode* pNode, E_Plot_Process_Type eProcessType, CProcessData<T>& data)
		{
			m_pProcessData = &data;
			DoProcess(pNode, eProcessType);
			m_pProcessData = 0;
		}

		/**  
		  * @brief 根据类型进入具体类型访问器进行不同的处理
		*/
		virtual bool VisitExit(FeExtNode::CExComposeNode& externNode);
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
	private:
		/**  
		  * @brief 属于一大类型军标的通用处理函数
		*/
		void CommonTypeHandle(FeExtNode::CExternNode& externNode){}
		void CommonTypeHandle(FeExtNode::CExFeatureNode& externNode);
		void CommonTypeHandle(FePlots::CLinePlot& externNode);
		void CommonTypeHandle(FePlots::CPolygonPlot& externNode);

		/**  
		  * @brief 获取要处理的具体数据
		*/
		template <class T>
		T* GetPlotData() 
		{
			CProcessData<T>* pData = dynamic_cast< CProcessData<T>* >(m_pProcessData); 
			return pData ? pData->m_pData : 0;
		}

	private:
		/// 处理类型
		E_Plot_Process_Type		m_eProcessStyle;

		/// 当前处理的数据
		CPlotDataBase*			m_pProcessData;

		// 军标是否绘制标识
		bool					m_bPlotFlag;
		
	};
}

#endif
