/**************************************************************************************************
* @file DistMeasure.h
* @note 距离测量的类，主要定义了测量距离的相关事宜
* @author l00008
* @data 2014-06-10
**************************************************************************************************/
#ifndef FE_DIST_MEASURE_H
#define FE_DIST_MEASURE_H

#include <FeMeasure/Measure.h>

#include <osgEarthAnnotation/PlaceNode>
#include <FeMeasure/BillBoardDisplay.h>

namespace FeMeasure
{
	/**
	  * @class CDistMeasureResultData
	  * @brief 距离测量结果数据
	  * @author g00034
	*/
	class CDistMeasureResultData : public CMeasureResultData
	{
	public:
		/**
		  *@note: 计算结果值
		*/
		double& Value(){ return m_dResultData; }
		const double& Value() const { return m_dResultData; }

	private:
		/// 测量结果值
		double    m_dResultData;
	};
}

namespace FeMeasure
{
	/**
	  * @class CDistMeasureResult
	  * @brief 距离测量结果基类
	  * @note 距离测量结果基类，定义了统一的距离测量所使用的接口
	  * @author g00034
	*/
    class CDistMeasureResult : public CMeasureResult
	{
	public:
		/**  
		  * @brief 析构函数
		*/
		virtual ~CDistMeasureResult(){}
		
	protected:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CDistMeasureResult(FeUtil::CRenderContext* pContext);
		
		/**
		*@note: 根据输入数据更新feature节点
		*/
		virtual void UpdateFeature(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 初始化显示节点
		*/
		bool InitFeatureNode();

		/**
		*@note: 数据成员读取函数，供派生类使用
		*/
		CDistMeasureResultData* GetData(){ return m_pData; }
		osgEarth::Features::Feature* GetFeature(){ return m_rpFeature.valid() ? m_rpFeature.get() : NULL; }
		osgEarth::Annotation::FeatureNode* GetFeatureNode(){ return m_rpFeatureNode.valid() ? m_rpFeatureNode.get() : NULL; }
		CMeasuringInfoDisplay* GetMeasureInfoDisplay(){ return m_rpMeasureInfoDisplay.valid() ? m_rpMeasureInfoDisplay.get() : NULL; }
	
	private:
		/**
		*@note: 根据输入点计算测量结果
		*/
		virtual bool CalculateImplement(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData(){m_pData = new CDistMeasureResultData; return m_pData; }

		/**
		*@note: 计算测量结果，派生类实现该函数进行不同的计算
		*/
		virtual bool DoCalculate() = 0;

		/**
		*@note: 创建feature，派生类实现该函数创建不同的feature
		*/
		virtual osgEarth::Features::Feature* CreateFeature() = 0;

	private:
		/// 用于距离测量的显示节点
        osg::ref_ptr< osgEarth::Annotation::FeatureNode > m_rpFeatureNode;
		osg::ref_ptr< osgEarth::Features::Feature >  m_rpFeature;
		
		/// 测量信息显示对象
		osg::ref_ptr<CMeasuringInfoDisplay>           m_rpMeasureInfoDisplay;

		/// 测量结果数据对象
		_ptr_observer(CDistMeasureResultData)         m_pData;
	};
}


#endif //FE_MEASURE_H



