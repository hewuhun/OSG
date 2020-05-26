/**************************************************************************************************
* @file CSpaceAreaMeasureOld.h
* @note 面积测量的类，主要定义了测量距离的相关事宜
* @author l00008
* @data 2014-06-10
**************************************************************************************************/
#ifndef FE_AREA_MEASURE_H
#define FE_AREA_MEASURE_H

#include <osgEarth/Containers>
#include <FeMeasure/DistMeasure.h>


namespace FeMeasure
{
	/// 结果数据
	typedef CDistMeasureResultData CSpaceAreaResultData;

	/**
	  * @class CSpaceAreaResult
	  * @note 空间面积测量结果类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSpaceAreaResult : public CDistMeasureResult
	{
	public:
		CSpaceAreaResult(FeUtil::CRenderContext* pContext);

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
	  * @class CSpaceAreaMeasure
	  * @note 空间面积测量类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSpaceAreaMeasure : public CMeasure
	{
		friend class CSpaceAreaFactory;

	private:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CSpaceAreaMeasure(FeUtil::CRenderContext* pContext);

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult();
	};
}


#endif //FE_AREA_MEASURE_H