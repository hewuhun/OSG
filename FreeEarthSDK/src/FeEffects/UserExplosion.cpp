#include <FeEffects/UserExplosion.h>
#include <FeUtils/CoordConverter.h>
#include <osg/Geode>

#include <FeUtils/PathRegistry.h>

namespace FeEffect
{
	CUserExplosionElement::CUserExplosionElement(FeUtil::CRenderContext* pContext) 
		: osg::MatrixTransform()
		, FeUtil::CRenderContextObserver(pContext)
		,m_vecPosition(osg::Vec3d(0, 0, 0))
	{
		CreateExplosion();
	}

	CUserExplosionElement::~CUserExplosionElement()
	{

	}


	osg::Group* CUserExplosionElement::CreateExplosion()
	{
		osg::ref_ptr<osg::Group> pRoot = new osg::Group();

		osgParticle::ParticleSystem *pExplpsionPS = CreateExplpsionPS(pRoot.get());
		osgParticle::ParticleSystem *pSmokePS = CreateSmokePS(pRoot.get());

		osg::ref_ptr<osgParticle::ParticleSystemUpdater> pPSU = new osgParticle::ParticleSystemUpdater();

		pPSU->addParticleSystem(pExplpsionPS);
		pPSU->addParticleSystem(pSmokePS);

		pRoot->addChild(pPSU.get());

		addChild(pRoot.get());

		osg::ref_ptr<osgParticle::ExplosionDebrisEffect> rpExplosionDebri = new osgParticle::ExplosionDebrisEffect(osg::Vec3(0, 0, 10.0), 1, 10);
		addChild(rpExplosionDebri.get());
		
		return pRoot.release();
	}

	osgParticle::ParticleSystem *CUserExplosionElement::CreateExplpsionPS(osg::Group *root)
	{

		osgParticle::Particle pexplosion;
		pexplosion.setLifeTime(1);

		pexplosion.setSizeRange(osgParticle::rangef(2.75f, 5.0f));
		pexplosion.setAlphaRange(osgParticle::rangef(1.0f, 0.1f));
		pexplosion.setColorRange(osgParticle::rangev4(
			osg::Vec4(1, 1, 1, 1), 
			osg::Vec4(1, 1, 1, 1)));
		pexplosion.setRadius(0.05f);
		pexplosion.setMass(0.05f);

		pexplosion.setTextureTileRange( 5,5, 0, 24);

		osg::ref_ptr<osgParticle::ParticleSystem> ps = new osgParticle::ParticleSystem;
		ps->setDefaultAttributes(FeFileReg->GetFullPath("texture/effects/flame.png"), false, false);


		osg::ref_ptr<osgParticle::ModularEmitter> emitter1 = new osgParticle::ModularEmitter;
		emitter1->setParticleSystem(ps.get());
		emitter1->setParticleTemplate(pexplosion);

		osg::ref_ptr<osgParticle::RandomRateCounter> counter1 = new osgParticle::RandomRateCounter;
		counter1->setRateRange(10, 15);
		emitter1->setCounter(counter1.get());

		osg::ref_ptr<osgParticle::SectorPlacer> placer = new osgParticle::SectorPlacer;
		placer->setCenter(osg::Vec3(0, 0, 3.0));
		placer->setRadiusRange(1.0, 3.0);
		placer->setPhiRange(0, 2 * osg::PI);
		emitter1->setPlacer(placer.get());

		osg::ref_ptr<osgParticle::RadialShooter> shooter = new osgParticle::RadialShooter;
		shooter->setInitialSpeedRange(0, 0);

		shooter->setInitialRotationalSpeedRange(osgParticle::rangev3(
			osg::Vec3(0, 0, -0.1),
			osg::Vec3(0, 0, 0.1)));
		emitter1->setShooter(shooter.get());
		root->addChild(emitter1.get());


		osg::ref_ptr<osgParticle::ModularProgram> program = new osgParticle::ModularProgram;
		program->setParticleSystem(ps.get());

		osg::ref_ptr<osgParticle::AccelOperator> op1 = new osgParticle::AccelOperator;
		op1->setAcceleration(osg::Vec3(0, 0, 2.0f));
		program->addOperator(op1.get());  

		root->addChild(program.get());

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		geode->addDrawable(ps.get());

		root->addChild(geode.get());

		emitter1->setStartTime(0.0);        
		emitter1->setLifeTime(1.0);       
		emitter1->setEndless(false);

		return ps;
	}

	osgParticle::ParticleSystem *CUserExplosionElement::CreateSmokePS(osg::Group *root)
	{

		osgParticle::Particle pexplosion;
		pexplosion.setLifeTime(3);

		pexplosion.setSizeRange(osgParticle::rangef(0.70f, 3.0f));
		pexplosion.setAlphaRange(osgParticle::rangef(0.5f, 1.0f));
		pexplosion.setColorRange(osgParticle::rangev4(
			osg::Vec4(1, 1, 1, 1), 
			osg::Vec4(1, 1, 1, 1)));
		pexplosion.setRadius(0.05f);
		pexplosion.setMass(0.05f);

		pexplosion.setTextureTileRange( 4,4, 0, 15);

		osg::ref_ptr<osgParticle::ParticleSystem> ps = new osgParticle::ParticleSystem;


		ps->setDefaultAttributes(FeFileReg->GetFullPath("texture/effects/animated_smoke.png"), false, false);


		osg::ref_ptr<osgParticle::ModularEmitter> emitter1 = new osgParticle::ModularEmitter;
		emitter1->setParticleSystem(ps.get());
		emitter1->setParticleTemplate(pexplosion);


		osg::ref_ptr<osgParticle::RandomRateCounter> counter1 = new osgParticle::RandomRateCounter;
		counter1->setRateRange(10, 20);
		emitter1->setCounter(counter1.get());

		osg::ref_ptr<osgParticle::SectorPlacer> placer = new osgParticle::SectorPlacer;
		placer->setCenter(osg::Vec3(0, 0, 4.0));
		placer->setRadiusRange(1.0, 1.0);
		placer->setPhiRange(0, 2 * osg::PI);
		emitter1->setPlacer(placer.get());

		osg::ref_ptr<osgParticle::RadialShooter> shooter = new osgParticle::RadialShooter;
		shooter->setInitialSpeedRange(0, 0);

		shooter->setInitialRotationalSpeedRange(osgParticle::rangev3(
			osg::Vec3(0, 0, -1),
			osg::Vec3(0, 0, 1)));
		emitter1->setShooter(shooter.get());

		root->addChild(emitter1.get());

		osg::ref_ptr<osgParticle::ModularProgram> program = new osgParticle::ModularProgram;
		program->setParticleSystem(ps.get());

		osg::ref_ptr<osgParticle::AccelOperator> op1 = new osgParticle::AccelOperator;
		op1->setAcceleration(osg::Vec3(0, 0, 2.0f));
		program->addOperator(op1.get());  

		root->addChild(program);

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		geode->addDrawable(ps.get());

		root->addChild(geode.get());


		emitter1->setStartTime(0.6);        
		emitter1->setLifeTime(10.0);       
		emitter1->setEndless(false);


		return ps.release();
	}

	void CUserExplosionElement::SetPosition( osg::Vec3d pos )
	{

		m_vecPosition = pos;
		osg::Matrix matrix;
		FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), pos, matrix);
		setMatrix(matrix);

	}


}

