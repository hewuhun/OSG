/**************************************************************************************************
* @file CRadialLineVisibleMeasure.h
* @note 视域分析测量的类，主要定义了测量距离的相关事宜
* @author l00008
* @data 2014-06-10
**************************************************************************************************/
#ifndef FE_RADIA_LINE_VISIBLE_MEASURE_H
#define FE_RADIA_LINE_VISIBLE_MEASURE_H

#include <FeMeasure/DistMeasure.h>
#include <FeMeasure/RadialLineOfSightNode.h>

namespace FeMeasure
{
	/**
	  * @class CRadialLineVisibleResult
	  * @brief 视域分析结果类
	  * @note 分装了视域分析的计算过程和显示结果
	  * @author g00034
	*/
    class CRadialLineVisibleResult : public CMeasureResult
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CRadialLineVisibleResult(FeUtil::CRenderContext* pContext);
	
	private:
		/**
		*@note: 根据输入点计算测量结果
		*/
		virtual bool CalculateImplement(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 计算最终结果。部分测量为了提高效率，不会实时更新，而只有在测量结束时才显示结果
		*/
		virtual bool FinalCalculateImplement();

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData(){ return NULL; }

	private:
		/// 视域分析效果节点
		osg::ref_ptr<FeMeasure::CRadialLineOfSightNode> m_rpSightNode;	
	};
}

namespace FeMeasure
{
	/**
	  * @class CRadialLineVisibleMeasure
	  * @note 视域分析类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CRadialLineVisibleMeasure : public CMeasure
	{
		friend class CRadialLineFactory;

	private:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CRadialLineVisibleMeasure(FeUtil::CRenderContext* pContext);

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult(){ return new CRadialLineVisibleResult(GetRenderContext()); }

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



#endif //FE_RADIA_LINE_VISIBLE_MEASURE_H
