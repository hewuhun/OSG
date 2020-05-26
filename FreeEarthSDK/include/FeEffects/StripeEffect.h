#ifndef STRIPE_EFFECT_H
#define STRIPE_EFFECT_H

#include <iostream>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Drawable>
#include <osg/MatrixTransform>
#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>

using namespace osg;

namespace FeEffect
{

	class FEEFFECTS_EXPORT CStripeEffect : public FeEffect::CFreeEffect
	{
	public:
		CStripeEffect(FeUtil::CRenderContext* pRenderContext);

		~CStripeEffect();
		/**
		* @brief 更新凝视相机
		* @note  设置完成各种参数后，调用更新完成；
		*/
		virtual bool UpdateEffect(int nNum);

		bool SetVetex(osg::Vec3dArray* pVertex);

	protected:
		bool CreateEffect();

	private:
		osg::Vec4d													m_vecColor;
		osg::ref_ptr<osg::Geode>									m_rpGnode;//面
		osg::ref_ptr<osg::Geometry>									m_rpGeom;
		osg::ref_ptr<osg::DrawArrays>						m_rpDrawElementsUShort;//侧面
		osg::observer_ptr<FeUtil::CRenderContext>		            m_opRenderContext;
	};

}
#endif //STRIPE_EFFECT_H
