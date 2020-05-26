#ifndef EFFECT_FIXEDSTARINGSCAN_EFFECT_H
#define EFFECT_FIXEDSTARINGSCAN_EFFECT_H
/*

卫星常规的扫瞄地球的类，
凝视相机

若对角度大小不满意，调整Scan.cpp中的10.0 
osg::Vec3d thirdPos = osg::Matrix(osg::Matrix::translate(m_pos) * osg::Matrix::rotate(osg::inDegrees(10.0), osg::Vec3d(1.0, 0.0, 0.0))).getTrans() - m_pos;

*/
#include <iostream>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeCallback>
#include <osg/Drawable>
#include <osg/MatrixTransform>

#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>
#include <FeExtNode/ExModelNode.h>

using namespace osg;

namespace FeEffect
{
    class PyramidNodeCallback;
    class CUpdateLenCallback;

#define MAXNUMWAVE 10
	class FEEFFECTS_EXPORT CFixedStaringScanEffect : public FeEffect::CFreeEffect
	{
		friend class PyramidNodeCallback;
		friend class CUpdateLenCallback;
	public:
		CFixedStaringScanEffect(FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT);
		
		//pos卫星位置-世界坐标，扫瞄角速度-度/秒
		CFixedStaringScanEffect(float dRadiu);
        bool CreateEffect();
		void SetRadiu(float dRadiu);
		///通过设置凝视相机的扫描角和步进角，从而移动凝视扫描波
		void SetRotate(osg::Vec3d posture);
		void SetRotate(osg::Matrix mat);
		void updataLength(osg::Vec3 scrpos,osg::Vec3 targetPos);
		/**
        * @brief 更新凝视相机
        * @note  设置完成各种参数后，调用更新完成；
        */
		virtual bool UpdateEffect();
	private:
		void initPram();
		void createNode();
		void updateUniform(osg::NodeVisitor* nv);
		
	private:
		float m_radius;
		float m_xAngle;
		float m_yAngle;
		osg::observer_ptr<osg::Geode> m_opGeode;
		typedef std::vector<osg::Vec3> PointList;
		PointList m_pointList;

		osg::observer_ptr<osg::Geometry> m_opLeftFace;
		osg::observer_ptr<osg::Geometry> m_opRightFace;
		osg::observer_ptr<osg::Geometry> m_opDownFace;
		osg::observer_ptr<osg::Geometry> m_opUpFace;
		osg::observer_ptr<osg::Geometry> m_opFrontFace;
		osg::observer_ptr<osg::Geometry> m_opOutLine;
		int m_numVert;
		float m_edgeLength;
		float m_waveWidth;
		osg::Vec4 m_baseColor;
		osg::Vec4 m_waveColor;
		int m_numWave;
		bool m_isSmooth;
		float m_startLength[MAXNUMWAVE];
		float m_waveStartOffset;
		float m_widthBetweenWave;
		float m_moveSpeed;
		osg::ref_ptr<PyramidNodeCallback> m_rpCullCallback;
		osg::ref_ptr<CUpdateLenCallback> m_rpUpdateLenCallback;
		double     m_dRadius;                        //半径
		osg::ref_ptr<osg::MatrixTransform> m_rpMT;   //变换矩阵

		osg::observer_ptr<FeUtil::CRenderContext>					m_opRenderContext;
		osg::observer_ptr<osg::MatrixTransform>						m_opMT;

	};

	class PyramidNodeCallback : public osg::NodeCallback
	{
	public:
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	};


	class CUpdateLenCallback : public osg::NodeCallback
	{
	public:
		CUpdateLenCallback(FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT,CFixedStaringScanEffect* pFixedScan);
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	protected:
		osg::observer_ptr<FeUtil::CRenderContext>					m_opRenderContext;
		osg::observer_ptr<osg::MatrixTransform>						m_opMT;
		osg::observer_ptr<CFixedStaringScanEffect>                  m_opFixedScan;
	};
}
#endif //EFFECT_FIXEDSTARINGSCAN_EFFECT_H
