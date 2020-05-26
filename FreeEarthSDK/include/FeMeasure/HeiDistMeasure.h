/**************************************************************************************************
* @file CHeiDistMeasureOld.h
* @note 高度测量的类，主要定义了测量距离的相关事宜
* @author l00008
* @data 2014-06-10
**************************************************************************************************/
#ifndef FE_HEI_DIST_MEASURE_H
#define FE_HEI_DIST_MEASURE_H

#include <FeMeasure/DistMeasure.h>


namespace FeMeasure
{
	/// 结果数据
	typedef CDistMeasureResultData CHeiDistResultData;

	/**
	  * @class CHeiDistResult
	  * @note 高度测量测量结果类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CHeiDistResult : public CDistMeasureResult
	{
	public:
		CHeiDistResult(FeUtil::CRenderContext* pContext);

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
		*@note: 创建高度显示 featureNode
		*/
		void InitHeiFeature();

		/**
		*@note: 重置 featureNode
		*/
		void ResetHeiFeature(const osg::Vec3d& vecPosLLH);

		/**
		*@note: 根据输入数据更新feature节点
		*/
		virtual void UpdateFeature(const osg::Vec3dArray& vecPointsLLH);

	private:
		/// 高度测量图形绘制节点
		osg::ref_ptr< osgEarth::Features::Feature >  m_rpHeiFeature;
		osg::ref_ptr< osgEarth::Annotation::FeatureNode > m_rpHeiFeatureNode;
	};
}

namespace FeMeasure
{
	/**
	  * @class CHeiDistMeasure
	  * @note 高度测量测量类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CHeiDistMeasure : public CMeasure
	{
		friend class CHeiDistFactory;

	private:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CHeiDistMeasure(FeUtil::CRenderContext* pContext);

	private: 
		virtual CMeasureResult* CreateMeasureResult();

		/**
		*@note: 添加控制点具体实现
		*/
		virtual void AddCtrlPointImplement(const osg::Vec3d& vecPosLLH);
		
		/**
		*@note: 设置控制点集具体实现
		*/
		virtual void SetCtrlPointsImplement(const osg::Vec3dArray& vecPointsLLH);
	};
}


#endif //FE_HEI_DIST_MEASURE_H
