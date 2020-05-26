/**************************************************************************************************
* @file CShadowAreaMeasure.h
* @note 投影面积测量的类
* @author w00040
* @data 2016-12-12
**************************************************************************************************/
#ifndef FE_SHADOW_AREA_MEASURE_H
#define FE_SHADOW_AREA_MEASURE_H

#include <osgEarth/Containers>
#include <FeMeasure/DistMeasure.h>


namespace FeMeasure
{
	/// 结果数据
	typedef CDistMeasureResultData CShadowAreaResultData; 

	/**
	  * @class CShadowAreaResult
	  * @note 投影面积测量结果类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CShadowAreaResult : public CDistMeasureResult
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CShadowAreaResult(FeUtil::CRenderContext* pContext);

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
	  * @class CShadowAreaMeasure
	  * @note 投影面积测量类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CShadowAreaMeasure : public CMeasure
	{
		friend class CShadowAreaFactory;
	private:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CShadowAreaMeasure(FeUtil::CRenderContext* pContext);

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult();
	};
}


#endif