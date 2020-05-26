/**************************************************************************************************
* @file MutableCylinder.h
* @note 形状不规则的圆柱体效果，可以设置两头的半径大小，以及颜色，同时具有颜色动态变化的效果
* @author c00005
* @data 2015-11-13
**************************************************************************************************/
#ifndef FE_MUTABLE_CYLINDER_H
#define FE_MUTABLE_CYLINDER_H

#include <osg/Drawable>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/NodeCallback>

#include <FeEffects/Export.h>

namespace FeEffect
{
	class CMutableCylinderCallback;

	/**
	  * @class CMutableCylinder
	  * @note 形状不规则的圆柱体效果，可以设置两头的半径大小，以及颜色，同时具有颜色动态变化的效果
	  * @author c00005
	  */
	class FEEFFECTS_EXPORT CMutableCylinder : public osg::Group
	{
	public:
		///构造函数
		CMutableCylinder();

		/**
		  * @note 构造函数，带详细参数
		  * @param posStart [in] 世界坐标下的圆柱效果的起点
		  * @param posEnd [in] 世界坐标下的圆柱效果的终点
		  * @param dStartRadius [in] 圆柱的起始点的半径
		  * @param dEndRadius [in] 圆柱的终止点的半径
		  * @param dStartAlpha [in] 圆柱的起始点的透明度
		  * @param dEndAlpha [in] 圆柱的终止点的透明度
		  * @param color [in] 圆柱效果的颜色值
		  * @return 返回值
		  */
		CMutableCylinder(osg::Vec3d posStart, osg::Vec3d posEnd,
			double dStartRadius, double dEndRadius,
			osg::Vec4d color = osg::Vec4d(0.0, 1.0, 1.0, 0.8),
			double dStartAlpha = 0.0, double dEndAlpha = 0.5,
			 bool bColorTrans = true);

		~CMutableCylinder();

	public:
		//设置起始点和获得终止点
		void SetPoint(osg::Vec3d posStart, osg::Vec3d posEnd);
		void GetPoint(osg::Vec3d &posStart, osg::Vec3d &posEnd);

		//设置起始透明度
		void SetAlpha(double dStartAlpha, double dEndAlpha);

		//设置颜色
		void SetColor(osg::Vec4d color);

		//设置颜色是否进行变换
		void SetColorTransform(bool bChanged);

		//设置多少帧对动态效果进行更新一次
		void SetFrameDelta(int nFrameDelta);

		//设置本效果共需要多少个圆组成圆柱以及每个圆需要多少个顶点构成
		void SetSection(int nSection);
		int GetSection();

		void SetSectionPoint(int nPoint);
		int GetSectionPoint();

		//设置开始半径和结束半径；
		void SetRadius(double dStartRadius,double dEndRadius);

	protected:
		//初始化
		void Init();

		//更新
		void Update();


	protected:
		//开始和结束点的位置
		osg::Vec3d m_vecStartPoint;
		osg::Vec3d m_vecEndPoint;

		//圆柱体开始和结束的圆形半径
		double      m_dStartRadius;
		double      m_dEndRadius;

		//圆柱体开始和结束的透明度,和颜色值
		double      m_dStartAlpha;
		double      m_dEndAlpha;
		osg::Vec4d  m_color;

		//颜色是否进行变换
		bool		m_bColorTrans;

		//多少帧进行一次动画的切换
		int			m_nFrameDelta;

		//该圆柱需要绘制多少个圆以及每个圆需要用多少个顶点
		int			m_nSection;
		int			m_nDensity;

		//是否在绘制回调的时候需要进行更新
		bool		m_bUpdate;
		bool		m_bUpdatePos;

		//场景节点
		osg::ref_ptr<osg::Geode>			m_rpGeode;
		osg::ref_ptr<osg::Geometry>			m_rpGeom;
		osg::ref_ptr<osg::MatrixTransform>  m_rpMatrixTransform;

		osg::ref_ptr<CMutableCylinderCallback> m_rpCallback;


		friend CMutableCylinderCallback;
	};

	class CMutableCylinderCallback : public osg::Drawable::DrawCallback
	{
	public:
		CMutableCylinderCallback(CMutableCylinder* pCylinder);

		~CMutableCylinderCallback();
	public:
		virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const;

	private:
		mutable int m_nCurrentFrame;

		osg::observer_ptr<CMutableCylinder> m_opCylinder;
	};
}
#endif //FE_MUTABLE_CYLINDER_H