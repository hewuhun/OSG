/**************************************************************************************************
* @file FloodAnalysisMeasure.h
* @note 淹没模拟分析
* @author c00005
* @data 2015-9-11
**************************************************************************************************/
#ifndef FE_FLOOD_MEASURE_H
#define FE_FLOOD_MEASURE_H

#include <osg/Uniform>
#include <osgEarth/Containers>

#include <FeMeasure/FloodSurface.h>
#include <FeMeasure/Measure.h>


namespace FeMeasure
{
	class CFloodAnalysisResult;
   
	/**
    * @class CFloodHeightCallback
    * @brief 淹没分析中水面不断上升的事件回调类
    * @note 继承自NodeCallback，实现在更新时回调淹没水面不断上升的效果
	* @author c00005
    */
	class FEMEASURE_EXPORT CFloodHeightCallback
		:public osg::NodeCallback
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pFloodMeasureResult [in] 测量结果对象
		*/
		CFloodHeightCallback(CFloodAnalysisResult* pFloodMeasureResult);
		~CFloodHeightCallback();

	public:
		/**
		  *@note: 设置暂停水面动画
		*/
		void SetPause( bool bPause ){ m_bPause = bPause; }
		
		/**
		*@note: 重置
		*/
		void Reset(){ m_bReset = true; }

	protected:
		/**
		*@note: 更新回调处理函数
		*/
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

	protected:
		/// 前一次更新时间
		double m_dPreTime;

		/// 是否暂停处理
		bool m_bPause;

		/// 是否重置
		bool m_bReset;

		/// 测量结果对象
		osg::observer_ptr<CFloodAnalysisResult> m_opFloodMeasureResult;
	
	};
}


namespace FeMeasure
{
	/**
	  * @class CFloodAnalysisResultData
	  * @note 淹没模拟分析结果数据
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CFloodAnalysisResultData : public CMeasureResultData
	{
	public:
		/// 最高点
		osg::Vec3d maxHeiPoint;

		/// 最低点
		osg::Vec3d minHeiPoint;

		/// 模拟动画中每次的变化高度
		double     dDeltaHei;

		/// 总面积
		double     dTotalArea;

		/// 水面覆盖面积
		double     dFloodArea;

		/// 当前水面高度
		double     dCurrentFloodHei;

		CFloodAnalysisResultData()
		{
			maxHeiPoint = osg::Vec3d(0.0, 0.0, -1000000.0);
			minHeiPoint = osg::Vec3d(0.0, 0.0, 1000000.0);
			dDeltaHei = 10.0;
			dTotalArea = 0.0;
			dFloodArea = 0.0;
			dCurrentFloodHei = 0.0;
		}
	};
}

namespace FeMeasure
{
	/**
	  * @class CFloodAnalysisResult
	  * @note 淹没模拟分析结果类
	  * @author g00034
	*/
    class CFloodAnalysisMeasure;
    class FEMEASURE_EXPORT CFloodAnalysisResult : public CMeasureResult
    {
		friend class CFloodAnalysisMeasure;
		friend class CFloodHeightCallback;

	public:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		  * @param pMeasure [in] 测量对象
		*/
		CFloodAnalysisResult(FeUtil::CRenderContext* pContext, CFloodAnalysisMeasure* pMeasure);

	private:
		/**
		*@note: 开始动态量算
		*/
		void StartDynamicCalculate();

		/**
		*@note: 开始动态量算
		*/
		void PauseDynamicCalculate();

		/**
		*@note: 停止动态量算
		*/
		void StopDynamicCalculate();

		/**
		*@note: 重置水淹的状态，回到水淹之前的效果
		*/
		void ResetFloodState();
	
		/**
		*@note: 设置每秒水面的上升高度
		*/
		void SetDeltaHeight(double dHei);

		/**
		*@note: 设置是否显示边界
		*/
		void SetBorderShow(bool bShow);
		
		/**
		*@note: 获取边界是否显示
		*/
		bool IsShowBorder();

		/**  
		  * @note 将淹没的水面抬高设定的高度值  
		  * @return 如果水面成功太高返回 true，否则如果水面停止返回 false
		*/
		bool FloodRaiseStep(double dHeight);

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
		virtual CMeasureResultData* CreateResultData(){ m_pData = new CFloodAnalysisResultData; return m_pData; }
		CFloodAnalysisResultData* GetData(){ return m_pData; }

		/**
		*@note: 创建绘制节点
		*/
		void InitFeatureNode();
	
    protected:
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
		virtual void CalculateContainPoints(osg::Vec3dArray* pArray, osg::Vec3d minLonLat, osg::Vec3d maxLonLat, osg::Vec3dArray* pOutArray, osg::Vec3d& maxHeiPoint, osg::Vec3d& minHeiPoint, int nIntervalNum = 50);

		/**  
		  * @brief 使用冒泡法对顶点进行排序
		  * @param pArray [in] pArray
		  * @return 无返回值
		*/
		void BubbleSort(osg::Vec3dArray* pArray);

		/**  
		  * @brief 使用二分法从顶点数组中查找指定高度的顶点
		  * @param pArray [in] 顶点数组
		  * @param dHeight [in] 指定高度
		  * @return 返回顶点索引
		*/
		int BinarySearch(osg::Vec3dArray* pArray, double dHeight);

	protected:
		/// 地表区域图形绘制节点
		osg::ref_ptr< osgEarth::Annotation::FeatureNode > m_rpFeatureNode;
		osg::ref_ptr< osgEarth::Features::Feature >  m_rpFeature;
        osg::ref_ptr<osgEarth::Symbology::Polygon>   m_rpPolygon;

		/// 水面模拟节点
		osg::ref_ptr<CFloodSurface> m_rpFloodSurface;

		/// 区域中包含的顶点
		osg::ref_ptr<osg::Vec3dArray> m_rpContainPonits;

		/// 水面更新回调
		osg::ref_ptr<CFloodHeightCallback>	m_rpFloodHeiCallback;

		/// 当前测量
		osg::observer_ptr<CFloodAnalysisMeasure> m_opMeasure;

		/// 测量结果数据
		_ptr_observer(CFloodAnalysisResultData)  m_pData;

		/// 是否使用用户定义的模拟步长
		bool		m_bUserDefineDelta;
	};
}

namespace FeMeasure
{
	/**
	  * @class CFloodAnalysisMeasure
	  * @note 淹没模拟分析类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CFloodAnalysisMeasure : public CMeasure
	{
		friend class CFloodFactory;
		friend class CFloodAnalysisResult;

	public:
		/**
		*@note: 开始动态量算
		*/
		void StartDynamicCalculate(){ if(m_opCurResult.valid()) m_opCurResult->StartDynamicCalculate(); }

		/**
		*@note: 开始动态量算
		*/
		void PauseDynamicCalculate(){ if(m_opCurResult.valid()) m_opCurResult->PauseDynamicCalculate(); }

		/**
		*@note: 停止动态量算
		*/
		void StopDynamicCalculate(){ if(m_opCurResult.valid()) m_opCurResult->StopDynamicCalculate(); }

		/**
		*@note: 重置水淹的状态，回到水淹之前的效果
		*/
		void ResetFloodState(){ if(m_opCurResult.valid()) m_opCurResult->ResetFloodState(); }
	
		/**
		*@note: 设置每秒水面的上升高度
		*/
		void SetDeltaHeight(double dHei){ if(m_opCurResult.valid()) m_opCurResult->SetDeltaHeight(dHei); }

		/**
		*@note: 设置是否显示边界
		*/
		void SetBorderShow(bool bShow){ if(m_opCurResult.valid()) m_opCurResult->SetBorderShow(bShow); }
		
		/**
		*@note: 获取边界是否显示
		*/
		bool IsShowBorder(){ return m_opCurResult.valid() ? m_opCurResult->IsShowBorder() : false; }

	private: 
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CFloodAnalysisMeasure(FeUtil::CRenderContext* pContext);

		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult();

	private:
		/// 测量结果对象
		osg::observer_ptr<CFloodAnalysisResult>  m_opCurResult;
	};
}

#endif //FE_FLOOD_MEASURE_H
