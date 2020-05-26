#ifndef SAR_EFFECT_H
#define SAR_EFFECT_H

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
	class  CSAREffectCallback : public osg::NodeCallback
	{
	public:
		CSAREffectCallback();

		~CSAREffectCallback();

	protected:
		virtual void operator()(osg::Node *pNode,osg::NodeVisitor *pNv);


	protected:


	};

	class FEEFFECTS_EXPORT CSAREffect : public FeEffect::CFreeEffect
	{
	public:
		CSAREffect(FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT);
		
		~CSAREffect();
		/**
        * @brief 更新凝视相机
        * @note  设置完成各种参数后，调用更新完成；
        */
		virtual bool UpdateEffect();
		
		void SetMinAngle(double dAngle);
		double GetMinAngle();
		void SetMaxAngle(double dAngle);
		double GetMaxAngle();
		void SetForwardAngle(double dAngle);
		double GetForwardAngle();
		void SetAftAngle(double dAngle);
		double GetAftAngle();
		void SetLenght(double dLenght);
		double GetLenght();
	private:
		bool CreateEffect();
		osg::Vec3d GetPointXYZByHit(osg::Vec3d vecXYZ,osg::Vec3d vecCenter);
		bool GetPointXYZifHit(osg::Vec3d &vecXYZ,osg::Vec3d vecCenter);
	private:

		osg::Vec4d													m_vecColor;
		osg::Vec4d													m_vecColorLine;
		osg::observer_ptr<FeUtil::CRenderContext>					m_opRenderContext;
		osg::observer_ptr<osg::MatrixTransform>						m_opMT;
		osg::ref_ptr<osg::Geode>									m_rpGnode;//面
		osg::ref_ptr<osg::Geometry>									m_rpGeom;
		osg::ref_ptr<osg::DrawElementsUShort>						m_rpDrawElementsUShort;//侧面
		
		osg::ref_ptr<osg::Geode>									m_rpForwardGeode;//面
		osg::ref_ptr<osg::Geometry>									m_rpForwardGeom;
		osg::ref_ptr<osg::DrawElementsUShort>						m_rpForwardDraw;//面

		osg::ref_ptr<osg::Geode>									m_rpAftGeode;//面
		osg::ref_ptr<osg::Geometry>									m_rpAftGeom;
		osg::ref_ptr<osg::DrawElementsUShort>						m_rpAftDraw;//面

		osg::ref_ptr<osg::Geode>									m_rpGnodeLine;//线
		osg::ref_ptr<osg::Geometry>									m_rpGeomLine;
		osg::ref_ptr<osg::DrawArrays>								m_rpDrawArraysBL;//底线
		osg::ref_ptr<osg::DrawArrays>								m_rpDrawArraysSL;//底线
		double                                                      m_dMinAngle;
		double                                                      m_dMaxAngle;
		double                                                      m_dForwardAngle;
		double                                                      m_dAftAngle;
		double                                                      m_dLenght;
		double                                                      m_dHeight;
	};

}
#endif //SAR_EFFECT_H
