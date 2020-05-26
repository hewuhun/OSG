/**************************************************************************************************
* @file CSpaceDistMeasure.h
* @note 直线距离测量的类，主要定义了测量距离的相关事宜
* @author l00008
* @data 2014-06-10
**************************************************************************************************/
#ifndef FE_LINE_DIST_MEASURE_H
#define FE_LINE_DIST_MEASURE_H

#include <FeMeasure/DistMeasure.h>

namespace FeMeasure
{
	/// 结果数据
	typedef CDistMeasureResultData CSpaceDistResultData;

	/**
	  * @class CSpaceDistResult
	  * @note 空间距离测量结果类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSpaceDistResult : public CDistMeasureResult
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CSpaceDistResult(FeUtil::CRenderContext* pContext);

	private:
		/**
		*@note: 计算测量结果
		*/
		virtual bool DoCalculate();

		/**
		*@note: 创建feature
		*/
		virtual osgEarth::Features::Feature* CreateFeature();
	};
}

namespace FeMeasure
{
	/**
	  * @class CSpaceDistMeasure
	  * @note 空间距离测量类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSpaceDistMeasure : public CMeasure
	{
		friend class CSpaceDistFactory;
	private:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CSpaceDistMeasure(FeUtil::CRenderContext* pContext);

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult();
	};
}


#endif //FE_LINE_DIST_MEASURE_H