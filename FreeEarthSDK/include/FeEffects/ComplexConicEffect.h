#ifndef COMPLEX_CONIC_EFFECT_H
#define COMPLEX_CONIC_EFFECT_H

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

using namespace osg;

namespace FeEffect
{
	class  CComplexConicEffectCallback : public osg::NodeCallback
	{
	public:
		CComplexConicEffectCallback();

		~CComplexConicEffectCallback();

	protected:
		virtual void operator()(osg::Node *pNode,osg::NodeVisitor *pNv);


	protected:


	};

	class FEEFFECTS_EXPORT CComplexConicEffect : public FeEffect::CFreeEffect
	{
	public:
		CComplexConicEffect(FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT);
		
		~CComplexConicEffect();
		/**
        * @brief 更新凝视相机
        * @note  设置完成各种参数后，调用更新完成；
        */
		virtual bool UpdateEffect();
		
		void SetNAngle(double dAngle);
		double GetNAngle();
		void SetFAngle(double dAngle);
		double GetFAngle();
		void SetBAngle(double dAngle);
		double GetBAngle();
		void SetEAngle(double dAngle);
		double GetEAngle();
		void SetLenght(double dLenght);
		double GetLenght();
	private:
		bool CreateEffect();
		osg::Vec3d GetPointXYZByHit(osg::Vec3d vecXYZ,osg::Vec3d vecCenter);
	private:

		osg::Vec4d													m_vecColor;
		osg::Vec4d													m_vecColorLine;
		osg::observer_ptr<FeUtil::CRenderContext>					m_opRenderContext;
		osg::observer_ptr<osg::MatrixTransform>						m_opMT;
		osg::ref_ptr<osg::Geode>									m_rpGnode;//面
		osg::ref_ptr<osg::Geometry>									m_rpGeom;
		osg::ref_ptr<osg::DrawArrays>								m_rpDrawArrays;//底面
		osg::ref_ptr<osg::DrawElementsUShort>						m_rpDrawElementsUShort;//侧面

		osg::ref_ptr<osg::Geode>									m_rpGnodeLine;//线
		osg::ref_ptr<osg::Geometry>									m_rpGeomLine;
		osg::ref_ptr<osg::DrawArrays>								m_rpDrawArraysBL;//底线
		osg::ref_ptr<osg::DrawArrays>								m_rpDrawArraysSL;//侧线
		double                                                      m_dNAngle;
		double                                                      m_dFAngle;
		double                                                      m_dBAngle;
		double                                                      m_dEAngle;
		double                                                      m_dLenght;
		double                                                      m_dHeight;
	};

}
#endif //COMPLEX_CONIC_EFFECT_H
