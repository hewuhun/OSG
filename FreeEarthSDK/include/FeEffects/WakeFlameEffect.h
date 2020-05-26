#ifndef WAKEFLAME_EFFECT_H
#define WAKEFLAME_EFFECT_H

#include <osg/Group>
#include <osgParticle/ExplosionEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularProgram>

#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>


namespace FeEffect
{
	class CWakeFlameEffect;
	class CWakeFlameBase;
	class FEEFFECTS_EXPORT CWakeFlameCallback : public osg::NodeCallback
	{
	public:
		CWakeFlameCallback(CWakeFlameBase* pWakeBase,osg::MatrixTransform* pMT);
		virtual ~CWakeFlameCallback();

	public:

		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

	protected:

		CWakeFlameBase							*m_pWakeBase;
		osg::MatrixTransform* m_pMT;

		osg::Vec3d								m_vecPos;
		osg::Vec3d								m_vecWorld;

	};

}

namespace FeEffect
{
	class FEEFFECTS_EXPORT CWakeFlameBase : public CFreeEffect
	{
	public:
		CWakeFlameBase();
		virtual ~CWakeFlameBase();
	public:
		virtual bool CreateEffect();

		virtual bool StartEffect();

		virtual bool ClearEffect();

	public:
		void SetMT(osg::MatrixTransform *pMT);

		void AddWakeCallBack();

		void RemoveWakeCallBack();
		
		void SetPos(osg::Vec3d vecPos);

	protected:

		void SetEffectEnable(bool bEnable);

	protected:
		osg::ref_ptr<osg::MatrixTransform>			m_rpMT;
		osg::ref_ptr<CWakeFlameCallback>			m_rpWakeCallback;

	};
}

namespace FeEffect
{
	/**
    * @class CWakeFlameEffect
    * @note 尾焰效果
    */
	class FEEFFECTS_EXPORT CWakeFlameEffect : public CFreeEffect
	{
	public:
		CWakeFlameEffect();
		virtual ~CWakeFlameEffect();

		virtual bool CreateEffect();
		virtual bool ClearEffect();
		void SetPos( osg::Vec3d vecPos );

		void SetAngle(double dTheta,double dPhi);

		void SetShooterSpeedRange(double dSpeed);

		void SetParticleNum(int nNum);

		void SetLifeTime(double dTime);

		osg::Vec3d GetPos();

		void UpdatePosAndAngle(osg::Matrix matrix,double dTheta,double dPhi);

		void SetWakeAbsoluteCenter(osg::Vec3d vecPos);

		void SetSizeRange(double dBegin,double dEnd);

		void SetColorRange(osg::Vec4d VecBegin,osg::Vec4d VecEnd);

		/**  
        * @note 设置参考帧类型
		* @param nType [in] 传入参考类型，0为RELATIVE_RF；1为ABSOLUTE_RF；
        */
		void SetReferenceFrame(int nType);


		void UpdataFlameEffect();

		void SetEmmitterEnable(bool bEnable);

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
		double m_dTime;
		double m_dBegin;
		double m_dEnd;
	};
}

#endif //WAKEFLAME_EFFECT_H