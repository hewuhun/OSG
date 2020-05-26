#ifndef _FE_WAKESMOKE_EFFECT_H
#define _FE_WAKESMOKE_EFFECT_H

#include <osg/Group>
#include <osgParticle/ExplosionEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularProgram>

#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>


//namespace FeEffect
//{
//	class CWakeSmokeEffect;
//	class FEEFFECTS_EXPORT CWakeFlameCallback : public osg::NodeCallback
//	{
//	public:
//		CWakeFlameCallback();
//		virtual ~CWakeFlameCallback();
//
//	public:
//
//		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
//
//
//	protected:
//
//		osg::Vec3d								m_vecPos;
//
//		osg::Vec3d								m_vecCurrPos;
//
//	};
//
//}

namespace FeEffect
{
	/**
    * @class CWakeSmokeEffect
    * @note 效果
    */
	class FEEFFECTS_EXPORT CWakeSmokeEffect : public CFreeEffect
	{
	public:
		CWakeSmokeEffect();
		virtual ~CWakeSmokeEffect();

		virtual bool CreateEffect();

		virtual bool UpdataEffect();

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
		void  SetGeoTileScale(osg:: Vec3d v);

		void  SetGeoTileRotate( double angle, const osg::Vec3f axis );

		void  SetGeoTileTrans( osg::Vec3d v);

		/**  
        * @note 设置参考帧类型
		* @param nType [in] 传入参考类型，0为RELATIVE_RF；1为ABSOLUTE_RF；
        */
		void SetReferenceFrame(int nType);
		osg::Geode * GetSmokeGeode();
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

		osg::ref_ptr<osg::Geode>							m_rpGeode;
		double m_dTime;
		double m_dBegin;
		double m_dEnd;

		osg::Vec4d m_vecBegin;
		osg::Vec4d m_vecEnd;
	};
}

#endif //_FE_WAKESMOKE_EFFECT_H