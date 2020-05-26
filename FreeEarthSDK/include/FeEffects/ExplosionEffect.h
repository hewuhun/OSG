#ifndef __FE_BANG__
#define __FE_BANG__

#include <FeEffects/Export.h>

#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeCallback>
#include <osg/Drawable>
#include <osg/ImageSequence>

#include <FeEffects/FreeEffect.h>


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

using namespace osg;

namespace FeEffect
{
	class  CImageSequence
	{
	public:
		static CImageSequence* Instance();

		static void Release();

		osg::ImageSequence * GetExplosionSequence();
		osg::ImageSequence * GetFlameSequence();

	protected:
		CImageSequence();
		~CImageSequence();
		
	protected:
		osg::ref_ptr<osg::ImageSequence> m_rpExplosionSequence;
		osg::ref_ptr<osg::ImageSequence> m_rpFlameSequence;
		static CImageSequence*     m_pInstance;       //单一的实例
	};

	class FEEFFECTS_EXPORT CBang : public CFreeEffect
	{
	public:
		CBang(float size);
		~CBang();
		void Play();
		void Stop();

	public:
		virtual bool CreateEffect();
		virtual bool UpdataEffect();
		virtual bool ClearEffect();
		void SetCenter( osg::Vec3d vecCenter );
		void SetSize(double dSize);

		osgParticle::ParticleSystem *CreateSmokePS(osg::Group *root);
		

	private:
		osg::ref_ptr<osg::ImageSequence> m_rpImageSequence;

		osg::ref_ptr<osg::Group> m_pImageGroup;
		osg::ref_ptr<osg::Group> m_pSmokeGroup;

		osg::ref_ptr<osg::MatrixTransform> m_rpMt;
		osg::ref_ptr<osg::Geometry> m_rpGeom;

		double m_dSize;
	};
}

#endif //__FE_BANG__