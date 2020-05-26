/**************************************************************************************************
* @file FloodSurface.h
* @note 水面绘制节点
* @author c00005
* @data 2015-9-11
**************************************************************************************************/

#ifndef FE_FLOOD_SURFACE_H
#define FE_FLOOD_SURFACE_H

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Uniform>
#include <osg/MatrixTransform>

#include <FeUtils/RenderContext.h>

namespace FeMeasure
{
	/**
	  * @class CFloatCallBack
	  * @brief 水面效果更新回调
	  * @author c00005
	*/
	class CFloatCallBack: public osg::Uniform::Callback
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CFloatCallBack(float src){m_fSrc=src;};
		~CFloatCallBack(){};

	public:
		virtual void operator() ( osg::Uniform* uniform, osg::NodeVisitor* nv )
		{
			uniform->set(m_fSrc);
		}
		float m_fSrc;
	};

	/**
	  * @class CFloodSurface
	  * @brief 水面效果绘制节点
	  * @author c00005
	*/
	class CFloodSurface: public osg::MatrixTransform
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pContext [in] 当前渲染上下文
		*/
		CFloodSurface(FeUtil::CRenderContext* pContext);
		~CFloodSurface();

	public:
		/**  
		  * @brief 重置效果
		*/
		void Reset();

		/**  
		  * @brief 上升水面
		  * @param dHei [in] 上升高度
		*/
		bool Raise(double dHei);

		/**  
		  * @brief 设置水面最大最小高度
		*/
		void SetHeight(double dMaxHei, double dMinHei);

		/**  
		  * @brief 获取水面当前高度
		*/
		double GetCurrentHeight();

		/**  
		  * @note 设置水面的坐标使用经纬度表示  
		*/
		void SetFloodCoords(osg::Vec3dArray* pLLHCoords);

		/**  
		  * @note 初始化水面  
		*/
		void InitFloodSurface();

		/**  
		  * @note 隐藏水面 
		*/
		void Hide();

		/**  
		  * @note 显示水面  
		*/
		void Show();

		/**  
		  * @note 显示边框  
		*/
		void ShowBorder(bool bShow = true);

		/**  
		  * @note 判断是否显示边框  
		*/
		bool IsShowBorder();

	protected:
		/// 最小高度
		double m_dMinHeight;

		/// 最大高度
		double m_dMaxHeight;

		/// 水面坐标数组
		osg::ref_ptr<osg::Vec3dArray> m_rpFloodCoords;

		/// 水面边框节点
		osg::ref_ptr<osg::Group>	  m_pWaterBorderRoot;

		/// 基础位置
		osg::Vec3d	m_basicPosition;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>   m_opRenderContext;
	};

}

#endif //FE_FLOOD_SURFACE_H