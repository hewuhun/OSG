/**************************************************************************************************
* @file ProfileAnalysisMeasure.h
* @note 剖面分析测量的基类
* @author c00005
* @data 2015-8-24
**************************************************************************************************/
#ifndef FE_PROFILE_ANALYSIS_MEASURE_H
#define FE_PROFILE_ANALYSIS_MEASURE_H

#include <FeMeasure/DistMeasure.h>

namespace FeMeasure
{
	/**
	  * @class CProfileAnalysisResultData
	  * @note 剖面分析结果数据
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CProfileAnalysisResultData : public CMeasureResultData
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CProfileAnalysisResultData()
		{
			m_unSampleNum = 100;
			m_dSurDistance = m_dLineDistance = m_dSampleDis = 0;
			m_pSamPoint = new osg::Vec3dArray;
		}

		/**  
		  * @brief 采样间距
		*/
		double& SampleDistance(){ return m_dSampleDis; }
		const double& SampleDistance() const { return m_dSampleDis; }

		/**  
		  * @brief 采样点的数量
		*/
		unsigned int& SampleNum(){ return m_unSampleNum; }
		const unsigned int& SampleNum() const { return m_unSampleNum; }

		/**  
		  * @brief 所有的采样关键点
		*/
		osg::Vec3dArray* SamplePoints() { return m_pSamPoint.get(); }
		const osg::Vec3dArray* SamplePoints() const { return m_pSamPoint.get(); }

		/**  
		  * @brief 高度最高的关键点
		*/
		osg::Vec3d& HighestPoint(){ return m_maxHeiPoint; }
		const osg::Vec3d& HighestPoint() const { return m_maxHeiPoint; }

		/**  
		  * @brief 高度最低的关键点
		*/
		osg::Vec3d& LowestPoint(){ return m_minHeiPoint; }
		const osg::Vec3d& LowestPoint() const { return m_minHeiPoint; }

		/**  
		  * @brief 地表总长度
		*/
		double& SurfaceDistance(){ return m_dSurDistance; }
		const double& SurfaceDistance() const { return m_dSurDistance; }

		/**  
		  * @brief 直线总长度
		*/
		double& LineDistance(){ return m_dLineDistance; }
		const double& LineDistance() const { return m_dLineDistance; }

	private:
		/// 用于保存所有的采样点、
		osg::ref_ptr<osg::Vec3dArray> m_pSamPoint; 

		/// 采样点
		unsigned int m_unSampleNum;   

		/// 采样点最小高度值
		osg::Vec3d   m_minHeiPoint;   

		/// 采样点最大高度值
		osg::Vec3d   m_maxHeiPoint;   

		/// 地表距离
		double       m_dSurDistance;  

		/// 直线距离
		double       m_dLineDistance; 

		/// 采样距离
		double       m_dSampleDis;    
		
	};
}

namespace FeMeasure
{
	/**
	  * @class CProfileAnalysisResult
	  * @note 剖面分析结果类
	  * @author g00034
	*/
    class FEMEASURE_EXPORT CProfileAnalysisResult : public CDistMeasureResult
    {
		friend class CProfileAnalysisMeasure;
		
	public:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CProfileAnalysisResult(FeUtil::CRenderContext* pContext);

	private:
		/**
		*@note: 计算测量结果
		*/
		virtual bool DoCalculate();

		/**
		*@note: 创建feature
		*/
		virtual osgEarth::Features::Feature* CreateFeature();

		/**
		*@note: 最终计算。为了提高效率，有些测量不会实时更新，而只有在最终才显示结果
		*/
		virtual bool FinalCalculateImplement();

		/**  
		  * @brief 设置采样点的数量
		*/
		void SetSamplePoint(unsigned int unNum);

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData(){ m_pData = new CProfileAnalysisResultData; return m_pData; }
		CProfileAnalysisResultData* GetData(){ return m_pData; }

	    /**  
		  * @brief 根据采样距离计算出每一个采样点
		  * @return double 本段距离中剩余的长度值
		*/
		double SubsectionArray(osg::Vec3d start, osg::Vec3d end, double dInternal, osg::Vec3dArray* pOutArray, double dDelta = 0.0);

	private:
		/// 测量结果数据对象
		_ptr_observer(CProfileAnalysisResultData)  m_pData;
    };
}

namespace FeMeasure
{
	/**
	  * @class CProfileAnalysisMeasure
	  * @note 剖面分析类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CProfileAnalysisMeasure : public CMeasure
	{
		friend class CProfileFactory;

	public:
		/**  
		  * @brief 设置采样点的数量
		*/
		void SetSamplePoint(unsigned int unNum);

	private: 
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CProfileAnalysisMeasure(FeUtil::CRenderContext* pContext);

		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult();

	private:
		/// 测量结果对象
		osg::observer_ptr<CProfileAnalysisResult>  m_opCurResult;
	};
}


#endif //FE_PROFILE_ANALYSIS_MEASURE_H