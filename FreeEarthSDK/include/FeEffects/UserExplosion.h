#ifndef USER_EXPLOSION_H
#define USER_EXPLOSION_H

#include <FeEffects/Export.h>

#include <osg/MatrixTransform>
#include <osgParticle/Particle>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/RadialShooter>
#include <osgParticle/AccelOperator>
#include <osgParticle/FluidFrictionOperator>
#include <osgParticle/ExplosionDebrisEffect>

#include <FeUtils/RenderContext.h>

namespace FeEffect
{
	class FEEFFECTS_EXPORT CUserExplosionElement : public osg::MatrixTransform, public FeUtil::CRenderContextObserver
	{
	public:
		CUserExplosionElement(FeUtil::CRenderContext* pContext);
		virtual ~CUserExplosionElement();
	public:
		//如果需要更多属性的设置，可以分别继承osg爆炸类获得默认粒子模板后自行设置。
		void SetPosition(osg::Vec3d pos);
	protected:
		osg::Group* CreateExplosion();
		osgParticle::ParticleSystem *CreateExplpsionPS(osg::Group *root);
		osgParticle::ParticleSystem *CreateSmokePS(osg::Group *root);
	protected:
		osg::Vec3d m_vecPosition;
	};
}

#endif //USER_EXPLOSION_H