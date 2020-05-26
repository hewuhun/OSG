/**************************************************************************************************
* @file FillCutAnalysisMeasure.h
* @note 填挖方测量
* @author c00005
* @data 2015-9-11
**************************************************************************************************/
#ifndef FE_FILL_CUT_MEASURE_H
#define FE_FILL_CUT_MEASURE_H

#include <FeMeasure/Export.h>
#include <FeMeasure/Measure.h>

namespace FeMeasure
{
	/**
	  * @class CFillCutAnalysisResultData
	  * @note 填挖方分析结果数据
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CFillCutAnalysisResultData : public CMeasureResultData
	{
	public:
		/// 填充面积
		double        dFillArea;      

		/// 削减面积 
		double        dCutArea;       

		/// 填充体积
		double        dFillVolume;    

		/// 削减体积
		double        dCutVolune;

		/// 参考面高度
		double        dReferenHei;    

		/// 最高点
		osg::Vec3d    vecMaxHeiPoint; 

		/// 最低点
		osg::Vec3d    vecMinHeiPoint; 
	};
}

namespace FeMeasure
{
	/**
	  * @class CFillCutAnalysisResult
	  * @note 填挖方分析结果类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CFillCutAnalysisResult : public CMeasureResult
	{
		friend class CFillCutAnalysisMeasure;

	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CFillCutAnalysisResult(FeUtil::CRenderContext* pContext);

	private:
		/**
		*@note: 根据输入点计算测量结果
		*/
		virtual bool CalculateImplement(const osg::Vec3dArray& vecPointsLLH);
		
		/**
		*@note: 最终计算。为了提高效率，有些测量不会实时更新，而只有在最终才显示结果
		*/
		virtual bool FinalCalculateImplement();

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData(){ m_pData = new CFillCutAnalysisResultData; return m_pData; }
		CFillCutAnalysisResultData* GetData(){ return m_pData; }

		/**
		*@note: 创建绘制节点
		*/
		void InitFeatureNode();

		/**  
		  * @note 设置基准面的高度
		*/
		void SetReferenceHeight(double dHei);

		/**  
		  * @brief 计算边界范围
		  * @param pArray [in] 顶点数组
		  * @param dMinLon [out] 最小经度
		  * @param dMaxLon [out] 最大经度
		  * @param dMinLat [out] 最小纬度
		  * @param dMaxLat [out] 最大纬度
		  * @return 无返回值
		*/
		virtual void CalculateScope(osg::Vec3dArray* pArray, double& dMinLon, double& dMaxLon, double& dMinLat, double& dMaxLat);

		/**  
		  * @brief 计算边界范围中包含的点
		  * @param pArray [in] 顶点数组
		  * @param minLonLat [in] 最小经纬度
		  * @param maxLonLat [in] 最大经纬度
		  * @param pOutArray [out] 计算输出的顶点数组
		  * @param maxHeiPoint [out] 最大高度
		  * @param minHeiPoint [out] 最小高度
		  * @param nIntervalNum [in] 内部采样密度，默认值为50
		  * @return 无返回值
		*/
		virtual void CalculateContainPoints(osg::Vec3dArray* pArray, osg::Vec3d minLonLat, osg::Vec3d maxLonLat, 
			osg::Vec3dArray* pOutArray, osg::Vec3d& maxHeiPoint, osg::Vec3d& minHeiPoint, int nIntervalNum = 50);
		
		/**  
		  * @note 更新绘制图形
		*/
		void UpdateCalculateFeature();

	private:
		/// 地表区域图形绘制节点
		osg::ref_ptr< osgEarth::Annotation::FeatureNode > m_rpFeatureNode;
		osg::ref_ptr< osgEarth::Features::Feature >       m_rpFeature;
		osg::ref_ptr<osgEarth::Symbology::Polygon>        m_rpPolygon;

		/// 裁剪区域图形绘制节点
		osg::ref_ptr< osgEarth::Annotation::FeatureNode > m_rpFillCutFNode;
		osg::ref_ptr< osgEarth::Features::Feature >       m_rpFillCutFeature;
		osg::ref_ptr<osgEarth::Symbology::Polygon>        m_rpFillCutPolygon;

		/// 边界区域图形绘制节点
		osg::ref_ptr< osgEarth::Annotation::FeatureNode > m_rpBorderLineFNode;
		osg::ref_ptr< osgEarth::Features::Feature >       m_rpBorderLineFeature;

		/// 计算结果值
		_ptr_observer(CFillCutAnalysisResultData)          m_pData;

		/// 是否使用用户设置的参考面高度
		bool                                              m_bUserDefRefHei;
	};
}

namespace FeMeasure
{
	/**
	  * @class CFillCutAnalysisMeasure
	  * @note 填挖方分析类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CFillCutAnalysisMeasure : public CMeasure
	{
		friend class CFillCutFactory;

	public:
		/**  
		  * @note 设置基准面的高度
		*/
		void SetReferenceHeight(double dHei);

	private:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CFillCutAnalysisMeasure(FeUtil::CRenderContext* pContext);

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult();

	private:
		/// 测量结果对象
		osg::observer_ptr<CFillCutAnalysisResult>  m_opResult;
	};
}


#endif //FE_FILL_CUT_MEASURE_H