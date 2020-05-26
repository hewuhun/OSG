/**************************************************************************************************
* @file BuildingsLinesMeasure.h
* @note 建筑群点对点通视分析
* @author g00034
* @data 2017-1-5
**************************************************************************************************/

#ifndef  HG_BUILDINGS_LINES_MEASURE_H
#define  HG_BUILDINGS_LINES_MEASURE_H 1

#include <osg/Group>

#include <FeUtils/RenderContext.h>

#include <FeMeasure/Export.h>
#include <FeMeasure/BillBoardDisplay.h>
#include <FeMeasure/Measure.h>


namespace FeMeasure
{
	/**
	  * @class CBuildingsLinesResult
	  * @brief 通视分析结果类
	  * @note 分装了通视分析的计算过程和显示结果
	  * @author g00034
	*/
    class CBuildingsLinesResult : public CMeasureResult
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CBuildingsLinesResult(FeUtil::CRenderContext* pContext);
	
	private:
		/**
		*@note: 根据输入点计算测量结果
		*/
		virtual bool CalculateImplement(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData(){ return NULL; }

		/**
		*@note: 判断指定索引的通视线段是否已经存在，每个目标点都对应一条通视线段
				索引值从0开始
		*/
		bool IsVisibleLineExist(unsigned int unIndex){ return (unIndex < m_listLines.size()); }

		/**
		*@note: 判断指定索引的标牌是否存在
				索引值从0开始
		*/
		bool IsBillboardExist(unsigned int unIndex){ return (unIndex < m_listBillBoardDisplay.size()); }

		/**
		*@note: 判断指定索引的标牌位置是否发生变化
				索引值从0开始
		*/
		bool IsBillboardPosNeedUpdate(unsigned int unIndex, const osg::Vec3d& vecLLH)
		{
			return ( IsBillboardExist(unIndex)
				&& (m_listBillBoardDisplay.at(unIndex).get()->GetPosition() != vecLLH) );
		}

		/**
		*@note: 判断目标点是否不需要更新, 如果对应的通视线，标牌都已经存在，且位置未发生变化，则不需要更新
		*/
		bool IsDestPointNotNeedUpdate(unsigned int unIndex, const osg::Vec3d& vecLLH)
		{
			return (IsVisibleLineExist(unIndex) && IsBillboardExist(unIndex+1) 
				&& !IsBillboardPosNeedUpdate(unIndex+1, vecLLH));
		}

		/**
		*@note: 创建指定索引目标点对应的通视线段
		*/
		bool CreateVisibleLine(unsigned int unIndex, osg::Vec3Array* pArrayVertex);

		/**  
		  * @note 创建指定索引目标点信息
		  * @param unIndex [in] 当前点的索引值
		  * @param @param vecPosLLH [in] 位置（单位：经纬高）
		  * @param distVisible [in] 可视距离
		  * @param distInvisible [in] 不可视距离
		  * @return 是否创建成功
		*/
		bool CreateTargetPointInfo(unsigned int unIndex, const osg::Vec3d& vecPosLLH, double distVisible, double distInvisible);

		/**  
		  * @brief 创建观察点信息
		  * @note 详细说明  
		  * @param unIndex [in] 当前点的索引值
		  * @param vecPosLLH [in] 观察点位置（单位：经纬高）
		  * @return 是否创建成功
		*/
		bool CreateLookPointInfo(unsigned int unIndex, const osg::Vec3d& vecPosLLH);

		/**
		*@note: 为测量点新增通视线段节点，并添加到通视线段列表
		*/
		bool AddNewVisibleLine();

		/**
		*@note: 处理目标点
		*/
		void ProcessLookPoint(const osg::Vec3d& lookPointLLH){ CreateLookPointInfo(0, lookPointLLH); }

		/**
		*@note: 处理目标点
		*/
		void ProcessDestPoints(const osg::Vec3d& lookPointXYZ, const osg::Vec3dArray& vecDestPointsLLH);

		/**
		*@note: 计算观察点与目标点的连线与地球的第一个交点位置
		*@return 如果有交点则返回第一个交点位置，无交点则返回目标点位置
		*/
		osg::Vec3d GetFirstIntersectPoint(const osg::Vec3d& lookPointXYZ, const osg::Vec3d& destPointLLH);

		/**
		*@note: 根据当前所需数量重置标牌，去除多余的标牌
		*/
		void ResetBillBoard(int numNeed);

		/**
		*@note: 根据当前所需数量重置通视线，去除多余的通视线
		*/
		void ResetVisibleLine(int numNeed);

	private:
		/// 标牌列表
		typedef std::vector< osg::ref_ptr<FeMeasure::CMeasuringInfoDisplay> > BILLBOARD_LIST;
		BILLBOARD_LIST m_listBillBoardDisplay;

		/// 连接观察点和目标点，并示意通视情况的线段列表
		typedef std::vector< osg::ref_ptr<osgEarth::Annotation::FeatureNode> > VISIBLE_LINE_LIST;
		VISIBLE_LINE_LIST m_listLines;
		
		/// 变换矩阵节点
		osg::ref_ptr<osg::MatrixTransform>   m_rpMtRoot;
	};
}

namespace FeMeasure
{
	/**
	  * @class CBuildingsLinesMeasure
	  * @note 建筑群点对点通视分析类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CBuildingsLinesMeasure : public CMeasure
	{
		friend class CIntervisibleFactory;

	private:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CBuildingsLinesMeasure(FeUtil::CRenderContext* pContext);

	private: 
		/**
		  *@note: 创建测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult(){ return new CBuildingsLinesResult(GetRenderContext()); }
	};
};


#endif
