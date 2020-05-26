/**************************************************************************************************
* @file RadialLineOfSightNode.h
* @note 视域分析特效
* @author w00024
* @data 2016-7-11
**************************************************************************************************/
#ifndef HG_RADIAL_LINE_OF_SIGHT_NODE_H
#define HG_RADIAL_LINE_OF_SIGHT_NODE_H

#include <osg/Group>

#include <osgEarth/GeoData>
#include <osgEarth/MapNode>
#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/FeatureNode>
#include <FeUtils/RenderContext.h>
#include <FeMeasure/BillBoardDisplay.h>

#include <FeMeasure/Export.h>

namespace FeMeasure
{
	class CRadialLineCallBack;
	
	/**
	* @class CRadialLineOfSightNode
	* @note 视域分析效果绘制节点
	* @author w00024 
	*/
	class FEMEASURE_EXPORT CRadialLineOfSightNode : public osg::Group
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pMapNode [in] 地球节点
		*/
		explicit CRadialLineOfSightNode(osgEarth::MapNode *mapNode);
		virtual ~CRadialLineOfSightNode();

		/**  
		  * @brief 设置中心点
		  * @param center [in] 中心点位置
		*/
		void SetCenter(const osgEarth::GeoPoint& center);
		const osgEarth::GeoPoint& GetCenter();

		/**  
		  * @brief 设置视点高度
		  * @param height [in] 高度值
		*/
		void SetViewHeight(double height);
		double GetViewHeight();

		/**  
		  * @brief 设置结束点位置
		  * @param endLLH [in] 结束点位置
		*/
		void SetEndLLH(osg::Vec3d endLLH);
		const osg::Vec3d& GetEndLLH();

		/**  
		  * @brief 计算网格图形
		*/
		void ComputeGrid();

		/**  
		  * @brief 更新网格绘制
		*/
		void UpdateGrid();

		/**  
		  * @brief 停止计算
		*/
		void StopCompute();

		/**  
		  * @brief 计算圆形网格图形
		*/
		void ComputeCircleGrid();

		/**  
		  * @brief 计算视线图形
		*/
		void ComputeViewLine();

	private:
		/**  
		  * @brief 创建边界框
		*/
		void CreateOutlineElement();

		/**  
		  * @brief 计算边界框
		*/
		void ComputeOutline();

		/**  
		  * @brief 更新边界框
		*/
		void UpdateOutline();

		/**  
		  * @brief 创建环状几何体线
		*/
		void CreateMouseLineElement();

		/**  
		  * @brief 更新环状几何体线
		*/
		void UpdateMouseLine();

		/**  
		  * @brief 创建视线图形
		*/
		void CreateViewLineElement();

		/**  
		  * @brief 创建网格图形
		*/
		void CreateGridElement();

		/**  
		  * @brief 重新生成绿色区域几何形状
		*/
		void RebuildGreenGeometry();

		/**  
		  * @brief 重新生成红色区域几何形状
		*/
		void RebuildRedGeometry();

	private:
		/// 地球节点
		osg::observer_ptr<osgEarth::MapNode> m_opMapNode;

		/// 内部绘制参数
		osgEarth::GeoPoint m_geoCenter;
		osg::Vec3d m_v3dEndLLH;
		double m_dViewHeight;
		int m_iNumSpokes;
		int m_iDrawStart, m_iDrawCur;
		int m_iDetectNum;
		bool m_bComputeCircleGrid, m_bComputeViewLine;

		/// 颜色数组
		typedef std::pair<bool, osg::Vec2d> ColorToPos;
		typedef std::vector<ColorToPos> VecColorToPos;
		std::vector<VecColorToPos> m_vecColorToPos;

		/// 边框图形节点
		osg::ref_ptr<osgEarth::Features::Feature>		m_rpOutlineFeature;
		//保存原始数据
		std::vector<osg::Vec3d>											m_vecData;
		osg::ref_ptr<osgEarth::Annotation::FeatureNode>	m_rpOutlineFeatureNode;

		/// 视线图形节点
		osg::ref_ptr<osgEarth::Features::Feature>       m_rpViewFeature;
		osg::ref_ptr<osgEarth::Annotation::FeatureNode> m_rpViewFeatureNode;
		osg::ref_ptr<osg::MatrixTransform>				m_rpViewSphere;

		/// 环状图形节点
		osg::ref_ptr<osgEarth::Features::Feature>       m_rpMouseLineFeature;
		osg::ref_ptr<osgEarth::Annotation::FeatureNode> m_rpMouseLineFeatureNode;

		/// 网格图形节点（绿色，通视部分）
		osg::ref_ptr<osgEarth::Features::MultiGeometry> m_rpGridMultiGeometryGreen;
		osg::ref_ptr<osgEarth::Features::Feature>       m_rpGridFeatureGreen;
		osg::ref_ptr<osgEarth::Annotation::FeatureNode> m_rpGridFeatureNodeGreen;

		/// 网格图形节点（红色，不可通视部分）
		osg::ref_ptr<osgEarth::Features::MultiGeometry> m_rpGridMultiGeometryRed;
		osg::ref_ptr<osgEarth::Features::Feature>       m_rpGridFeatureRed;
		osg::ref_ptr<osgEarth::Annotation::FeatureNode> m_rpGridFeatureNodeRed;

		/// 更新回调
		osg::ref_ptr<CRadialLineCallBack>				m_rpRadialCallBack;

		/// 标牌显示对象
		osg::ref_ptr<FeMeasure::CMeasuringInfoDisplay>  m_rpMeasureInfoDisplay;
	};

	/**
	* @class CRadialLineCallBack
	* @note 视域线段回调，主要用于更新视域效果节点的绘制
	* @author w00024 
	*/
	class FEMEASURE_EXPORT CRadialLineCallBack
		:public osg::NodeCallback
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pSightNode [in] 视域分析效果节点
		*/
		explicit CRadialLineCallBack(CRadialLineOfSightNode* pSightNode);
		~CRadialLineCallBack();

	protected:
		/**  
		  * @brief 更新回调函数，主要用于更新效果节点的绘制
		*/
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

	protected:
		/// 视域分析效果节点
		osg::observer_ptr<CRadialLineOfSightNode> m_opSightNode;
	};
}



#endif // !HG_RADIAL_LINE_OF_SIGHT_NODE_H
