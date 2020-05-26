/**************************************************************************************************
* @file CSurfDistMeasureOld.h
* @note 定义了表面距离测量
* @author l00008
* @data 2014-06-10
**************************************************************************************************/
#ifndef FE_SURF_DIST_MEASURE_H
#define FE_SURF_DIST_MEASURE_H

#include <osgText/Text>

#include <FeMeasure/DistMeasure.h>
#include <FeKits/ScreenKits.h>
#include <FeKits/KitsCtrl.h>


namespace FeMeasure
{
	/// 结果数据
	typedef CDistMeasureResultData CSurfDistResultData;

	/**
	  * @class CSurfDistResult
	  * @note 地表距离测量结果类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSurfDistResult : public CDistMeasureResult
	{
	public:
		CSurfDistResult(FeUtil::CRenderContext* pContext);

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
	  * @class CSurfDistMeasure
	  * @note 地表距离测量类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSurfDistMeasure : public CMeasure
	{
		friend class CSurfaceDistFactory;

	private:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CSurfDistMeasure(FeUtil::CRenderContext* pContext);

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult();
	};
}


#endif //FE_SURF_DIST_MEASURE_H