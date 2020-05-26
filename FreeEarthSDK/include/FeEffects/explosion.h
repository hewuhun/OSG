#ifndef _EXPLOSION_H
#define _EXPLOSION_H

#include <FeEffects/Export.h>

#include <osg/MatrixTransform>
#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

#include <FeUtils/RenderContext.h>

namespace FeEffect
{
	class FEEFFECTS_EXPORT CExplosionElement : public osg::MatrixTransform, public FeUtil::CRenderContextObserver
	{
	public:
		CExplosionElement(FeUtil::CRenderContext* pContext);
		virtual ~CExplosionElement();
	public:
		//如果需要更多属性的设置，可以分别继承osg爆炸类获得默认粒子模板后自行设置。
		void SetPosition(osg::Vec3d pos);
		void SetScale(double scale);
		void SetIntensity(double intensity);
		void SetWind(osg::Vec3d wind);
	protected:
		osg::Group* CreateExplosion();
	protected:
		osg::Vec3d m_vecPosition;
		osg::Vec3d m_vecWind;
		double m_dScale;
		double m_dIntensity;
		osg::ref_ptr<osgParticle::ExplosionEffect> m_rpExplosion;
		osg::ref_ptr<osgParticle::ExplosionDebrisEffect> m_rpExplosionDebri;
		osg::ref_ptr<osgParticle::FireEffect> m_rpFire;
		osg::ref_ptr<osgParticle::ParticleEffect> m_rpSmoke;
	};
}

#endif