#ifndef WAKEFLAME_EFFECT_H
#define WAKEFLAME_EFFECT_H

#include <osg/Group>
#include <osgParticle/ExplosionEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularProgram>

#include <FeUtils/Export.h>
#include <FeExNode/FreeEffect.h>

#include <FeUtils/MiniAnimationCtrl.h>

namespace FeUtil
{
	class CWakeFlameEffect;
	class CWakeFlameBase;
	class FEUTIL_EXPORT CWakeFlameCallback : public osg::NodeCallback
	{
	public:
		CWakeFlameCallback(CMiniAnimationCtrl *pCtrl,CWakeFlameBase* pWakeBase);
		virtual ~CWakeFlameCallback();

	public:

		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

		void SetMiniAnimationCtrl(CMiniAnimationCtrl *pCtrl);

	protected:

		CMiniAnimationCtrl						*m_pCtrl;

		CWakeFlameBase							*m_pWakeBase;

		osg::Vec3d								m_vecPos;

	};

}

namespace FeUtil
{
	class FEUTIL_EXPORT CWakeFlameBase : public FeExNode::CFreeEffect
	{
	public:
		CWakeFlameBase();
		virtual ~CWakeFlameBase();

		virtual bool CreateEffect();

		void SetMiniAnimationCtrl(CMiniAnimationCtrl *pCtrl);
	protected:
		osg::ref_ptr<osg::MatrixTransform>			m_rpMT;
		osg::ref_ptr<CWakeFlameCallback>			m_rpWakeCallback;
		osg::ref_ptr<CMiniAnimationCtrl>			m_rpCtrl;

	};
}

namespace FeUtil
{
	class FEUTIL_EXPORT CWakeFlameEffect : public FeExNode::CFreeEffect
	{
	public:
		CWakeFlameEffect();
		virtual ~CWakeFlameEffect();

		virtual bool CreateEffect();

		void SetPos( osg::Vec3d vecPos );

		void SetAngle(double dTheta,double dPhi);

		void SetShooterSpeedRange(double dSpeed);
		void SetParticleNum(int nNum);
		void SetLifeTime(double dTime);
		osg::Vec3d GetPos();
		void SetMiniAnimationCtrl(CMiniAnimationCtrl *pCtrl);

		void UpdatePosAndAngle(osg::Matrix matrix,double dTheta,double dPhi);
		void SetWakeAbsoluteCenter(osg::Vec3d vecPos);
		void SetSizeRange(double dBegin,double dEnd);
		void SetColorRange(osg::Vec4d VecBegin,osg::Vec4d VecEnd);
	protected:
		osg::Vec3d       m_vecPos;

		osg::ref_ptr<osgParticle::ParticleSystemUpdater>	m_rpPSU;
		osg::ref_ptr<osgParticle::ParticleSystem>			m_rpPS;
		osgParticle::Particle								m_ParticleTemplate;
		osg::ref_ptr<osgParticle::ModularEmitter>			m_rpEmmitter;
		osg::ref_ptr<osgParticle::RadialShooter>			m_rpRShooter;
		osg::ref_ptr<osgParticle::PointPlacer>				m_rpPlacer;
		osg::ref_ptr<osgParticle::ModularProgram>			m_rpProgram;
		osg::ref_ptr<osgParticle::RandomRateCounter>		m_rpCounter;

		osg::ref_ptr<CMiniAnimationCtrl>					m_rpCtrl;

	};
}

#endif //WAKEFLAME_EFFECT_H