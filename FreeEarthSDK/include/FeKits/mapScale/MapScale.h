/**************************************************************************************************
* @file MapScale.h
* @note 比例尺挂件
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef FE_MAP_SCALE_H
#define FE_MAP_SCALE_H

#include <osg/NodeVisitor>
#include <osgText/Text>
#include <osgViewer/View>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

#include <osgEarth/MapNode>

#include <FeKits/ScreenKits.h>

#include <FeKits/Export.h>

namespace FeKit
{
	/**
	* @class CMeasureScale
	* @brief 一个显示当前地图比例尺的类
	* @note 通过实时的计算屏幕两坐标之间的距离来确定当前地图的比例尺，可以提供数字表示
	*       形式和比例形式的比例尺显示方案。
	* @author l00008
	*/
	class FEKIT_EXPORT CMapScale : public CScreenKits
	{
	public:
		/**
		* @brief 创建一个比例尺
		* @note 通过提供比例尺的起始屏幕坐标、宽度以及起始位置偏移来创建比例尺
		* @param pView [in]（添加比例尺的View）
		* @param nXOffset [in] X偏移
		* @param nYOffset [in] Y偏移
		* @param nWidth [in] 宽度
		* @param nHeight [in] 高度
		* @param ePlace [in] 位置
		*/
		CMapScale(
			FeUtil::CRenderContext* pContext,
			int nXOffset, 
			int nYOffset, 
			int nWidth, 
			int nHeight, 
			CPlaceStrategy::KIT_PLACE ePlace);

		/**
		@note: 更新两个点测量的距离
		*/
		void UpdateScale();

		/**
		@note: 获取距离
		*/
		double GetDistance();

	protected:
		~CMapScale(void);

		/**
		@note: 创建比例尺挂件
		*/
		osg::Geode* CreateMSUI();

		/**
		@note: 遍历，用于实时更新比例尺距离
		*/
		virtual void traverse(osg::NodeVisitor& nv);

	protected:
		///单位距离千米数字形式显示
		osg::observer_ptr<osgText::Text>        m_opDigtalMS;      

		///单位距离实际距离比例形式显示
		osg::observer_ptr<osgText::Text>        m_opRatioMS;        

		///开始测量位置
		osg::Vec3d                              m_vecStartPosRate;  

		///结束测量位置
		osg::Vec3d                              m_vecEndPosRate;    

		///数字比例尺显示位置
		osg::Vec3d                              m_vecDigtalRate;    

		///单位距离
		double                                  m_dDistance;       

		///清楚界面上的比例尺
		bool									m_bClear;   

		///渲染节点
		osg::ref_ptr<osg::Geode>				m_rpMapScaleNode; 
	};
}

#endif //FE_AIR_VIEW_H
