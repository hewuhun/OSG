/**************************************************************************************************
* @file GradientAnalysisMeasure.h
* @note 坡度坡向分析
* @author g00034
* @data 2017-1-5
**************************************************************************************************/

#ifndef  HG_GRADIENT_ANALYSIS_MEASURE_H
#define  HG_GRADIENT_ANALYSIS_MEASURE_H 1

#include <FeUtils/RenderContext.h>

#include <FeMeasure/Measure.h>

#include <FeMeasure/GradientMapEffect.h>
#include <FeMeasure/GradientBillBoardDisplay.h>
#include <FeMeasure/BillBoardDisplay.h>


namespace FeMeasure
{
	/**
	  * @class CGradientAnalysisResult
	  * @brief 坡度坡向分析结果类
	  * @note 分装了坡度坡向分析的计算过程和显示结果
	  * @author g00034
	*/
    class CGradientAnalysisResult : public CMeasureResult
	{
		friend class CGradientAnalysisMeasure;

	public:
		/**  
		  * @brief 构造函数
		  * @param pFloodMeasureResult [in] 测量结果对象
		*/
		CGradientAnalysisResult(FeUtil::CRenderContext* pContext);
		~CGradientAnalysisResult();

	private:
		/**
		*@note: 根据输入点计算测量结果
		*/
		virtual bool CalculateImplement(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData(){ return NULL; }

		/**
		*@note: 显示坡度分析结果
		*/
		void ShowSlopeAnalysisResult(osg::Vec3d llh);

		/**
		*@note: 创建显示节点
		*/
		void CreateElement();

		/**
		*@note: 处理位置信息
		*/
		void DealPositionInformation();

		/**
		*@note: 从计算结果中获取指定点的坡度坡向信息。成功返回ture，否则返回false
		*/
		bool GetGradientInfo(const osg::Vec3d& posLLH, float& fSlope, float& fAspect);

	private:
		/// 是否加载了效果
		bool m_bInstallEffect;

		/// 地球节点
		osg::observer_ptr<osgEarth::MapNode> m_opMapNode;

		/// 边界区域绘制节点
		osg::observer_ptr<osgEarth::Annotation::FeatureNode> m_opFeatureNode;
		osg::observer_ptr<osgEarth::Features::Feature> m_opFeature;
		osg::observer_ptr<osgEarth::Symbology::Ring> m_opLinePoints;

		/// 地形的坡度特效
		osg::ref_ptr<FeMeasure::CGradientMapEffect> m_rpGradientMap;

		/// 标牌显示节点
		osg::ref_ptr<FeMeasure::CGradientBillBoardDisplay> m_rpGradientBillBoard;
	};
}

namespace FeMeasure
{
	/**
	  * @class CGradientAnalysisMeasure
	  * @note 坡度坡向分析类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CGradientAnalysisMeasure : public CMeasure
	{
		friend class CGradientFactory;
	
	public:
		/**
		*@note: 处理鼠标移动事件
		*/
		virtual void traverse(osg::NodeVisitor& nv);

	private:
		/**  
		  * @brief 构造函数
		  * @param pFloodMeasureResult [in] 测量结果对象
		*/
		CGradientAnalysisMeasure(FeUtil::CRenderContext* pContext);

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult(){ return new CGradientAnalysisResult(GetRenderContext()); }

		/**
		*@note: 添加控制点具体实现
		*/
		virtual void AddCtrlPointImplement(const osg::Vec3d& vecPosLLH);
		
		/**
		*@note: 设置控制点集具体实现
		*/
		virtual void SetCtrlPointsImplement(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 显示坡度坡向信息
		*/
		void ShowGradientInfo(const osg::Vec3d& vecPosLLH);

	private:
		/// 测量信息显示对象
		osg::ref_ptr<CMeasuringInfoDisplay>           m_rpMeasureInfoDisplay;
	};
};


#endif
