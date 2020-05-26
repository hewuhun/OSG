#include <FeEffects/ExplosionEffect.h>
#include <FeUtils/PathRegistry.h>

#include <osg/PrimitiveSet>
#include <osg/AnimationPath>
#include <osg/LineWidth>
#include <osgText/Text>
#include <sstream>
#include <osg/Texture2D>
#include <osg/Billboard>
#include <osg/Depth>
#include <osgDB/ReadFile>
#include <osg/Billboard>
#include <iostream>
#include <iomanip>

#include <osgParticle/SmokeEffect>

using namespace osg;

namespace FeEffect
{
	CBang::CBang(float size)
		:m_rpImageSequence(NULL)
		,m_pImageGroup(NULL)
		,m_rpMt(NULL)
		,m_pSmokeGroup(NULL)
		,m_dSize(size)
		,m_rpGeom(NULL)
	{
	
		m_pImageGroup = new osg::Group();
		m_pSmokeGroup = new osg::Group();

		m_rpImageSequence = new osg::ImageSequence();

		m_rpGeom = osg::createTexturedQuadGeometry(
			osg::Vec3(-m_dSize/2.0, 0.0f, -m_dSize/2.0),
			osg::Vec3(m_dSize, 0.0f, 0.0f),
			osg::Vec3(0.0f, 0.0f, m_dSize));
	

		unsigned int maxNum = CImageSequence::Instance()->GetExplosionSequence()->getNumImageData();

		for(unsigned int n =0 ;n<maxNum;n++)
		{
			m_rpImageSequence->setImage(n,CImageSequence::Instance()->GetExplosionSequence()->getImage(n));
		}
		m_rpImageSequence->setLength(double(maxNum)*(1.0/30.0));
		m_rpImageSequence->setLoopingMode(osg::ImageStream::NO_LOOPING);

		osg::ref_ptr<osg::Texture2D>  pTexture = new osg::Texture2D;
		pTexture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
		pTexture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
		pTexture->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);
		pTexture->setResizeNonPowerOfTwoHint(false);
		pTexture->setImage(m_rpImageSequence.get());

		osg::ref_ptr<osg::StateSet> pStateset2 = new osg::StateSet;
		pStateset2->setTextureAttributeAndModes(0,pTexture,osg::StateAttribute::ON);
		pStateset2->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		pStateset2->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		osg::ref_ptr<osg::Depth> dp = new osg::Depth();
		dp->setWriteMask(false);
		this->getOrCreateStateSet()->setAttribute(dp.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		osg::ref_ptr<osg::Billboard> center = new osg::Billboard();
		center->setMode(osg::Billboard::POINT_ROT_EYE);
		center->addDrawable(m_rpGeom);
		center->setStateSet(pStateset2.get());

		center->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);


		if(!m_rpMt.valid())
		{
			m_rpMt = new osg::MatrixTransform();
		}
		
		m_rpMt->setMatrix(osg::Matrix::translate(m_vecCenter));
		m_rpMt->addChild(m_pImageGroup.get());
		m_pImageGroup->addChild(center.get());
		m_rpMt->addChild(m_pSmokeGroup.get());
		addChild(m_rpMt.get());
	}

	void CBang::SetCenter( osg::Vec3d vecCenter )
	{
		CFreeEffect::SetCenter(vecCenter);
		if(m_rpMt.valid())
		{
			m_rpMt->setMatrix(osg::Matrix::translate(vecCenter));
		}

	}

	void CBang::SetSize(double dSize)
	{
		m_rpGeom = osg::createTexturedQuadGeometry(
			osg::Vec3(-m_dSize / 2.0, 0.0f, -m_dSize / 2.0),
			osg::Vec3(m_dSize, 0.0f, 0.0f),
			osg::Vec3(0.0f, 0.0f, m_dSize));
	}

	void CBang::Play()
	{
		m_rpImageSequence->rewind();
		m_rpImageSequence->play();
	}

	void CBang::Stop()
	{
		m_rpImageSequence->pause();
		m_rpImageSequence->rewind();
	}

	CBang::~CBang()
	{

	}

	bool CBang::CreateEffect()
	{
		return true;
	}

	bool CBang::UpdataEffect()
	{
		Play();

		//osgParticle::ParticleSystem *pSmokePS = CreateSmokePS(this);
		//osg::ref_ptr<osgParticle::ParticleSystemUpdater> pPSU = new osgParticle::ParticleSystemUpdater();
		//pPSU->addParticleSystem(pSmokePS);
		//this->addChild(pPSU.get());

#if 0 //爆炸后烟效果
		m_pSmokeGroup->removeChildren(0,m_pSmokeGroup->getNumChildren());
		osg::Vec3d pos = m_vecCenter;
		pos.z() = -pos.z();
		osg::ref_ptr<osgParticle::SmokeEffect> pSmoke = new osgParticle::SmokeEffect(pos, 6.0);
		pSmoke->setParticleDuration(10.0);
		pSmoke->getParticleSystem()->getDefaultParticleTemplate().setColorRange(osgParticle::rangev4(
			osg::Vec4(0.5, 0.5, 0.5, 1), 
			osg::Vec4(1, 1, 1, 1)));
		m_pSmokeGroup->addChild(pSmoke.get());
		pSmoke->setUseLocalParticleSystem(false);
		pSmoke->getEmitter()->setLifeTime(10.0);
		pSmoke->getEmitter()->setEndless(false);

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		pSmoke->getParticleSystem()->setDefaultAttributes(FeFileReg->GetFullPath("texture/effects/smoke.rgb"), false, false);
		geode->addDrawable(pSmoke->getParticleSystem());
		m_pSmokeGroup->addChild(geode.get());
#endif
		return true;
	}

	osgParticle::ParticleSystem *CBang::CreateSmokePS(osg::Group *root)
	{

		osgParticle::Particle pexplosion;
		pexplosion.setLifeTime(3);

		pexplosion.setSizeRange(osgParticle::rangef(0.70f, 3.0f));
		pexplosion.setAlphaRange(osgParticle::rangef(0.5f, 1.0f));
		pexplosion.setColorRange(osgParticle::rangev4(
			osg::Vec4(0.2f, 0.2f, 0.2f, 1),
			osg::Vec4(1, 1, 1, 1) 
			));
		pexplosion.setRadius(0.05f);
		pexplosion.setMass(0.1f);

		pexplosion.setTextureTileRange( 4,4, 0, 15);

		osg::ref_ptr<osgParticle::ParticleSystem> ps = new osgParticle::ParticleSystem;


		ps->setDefaultAttributes(FeFileReg->GetFullPath("texture/effects/animated_smoke.png"), false, false);


		osg::ref_ptr<osgParticle::ModularEmitter> emitter1 = new osgParticle::ModularEmitter;
		emitter1->setParticleSystem(ps.get());
		emitter1->setParticleTemplate(pexplosion);
		//emitter1->setReferenceFrame(osgParticle::ParticleProcessor::ABSOLUTE_RF);


		osg::ref_ptr<osgParticle::RandomRateCounter> counter1 = new osgParticle::RandomRateCounter;
		counter1->setRateRange(10, 20);
		emitter1->setCounter(counter1.get());

		osg::ref_ptr<osgParticle::SectorPlacer> placer = new osgParticle::SectorPlacer;
		osg::Vec3d pos(0.0,0.0,4.0);
		
		placer->setCenter(pos);
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
		op1->setAcceleration(osg::Vec3(-3,0,  0.0f));
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

	bool CBang::ClearEffect()
	{
		removeChildren(0,this->getNumChildren());
		return false;
	}

}


namespace FeEffect
{
	CImageSequence* CImageSequence::m_pInstance = NULL;

	CImageSequence* CImageSequence::Instance()
	{
		if( !m_pInstance )
		{
			if( !m_pInstance)
			{
				m_pInstance = new CImageSequence();
			}
		}
		return m_pInstance;	
	}

	void CImageSequence::Release()
	{
		if(m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

	osg::ImageSequence * CImageSequence::GetExplosionSequence()
	{
		return m_rpExplosionSequence.get();
	}

	osg::ImageSequence * CImageSequence::GetFlameSequence()
	{
		return m_rpFlameSequence.get();
	}

	CImageSequence::CImageSequence()
		:m_rpExplosionSequence(NULL)
		,m_rpFlameSequence(NULL)
	{
		m_rpExplosionSequence = new osg::ImageSequence;


		for(int i = 1; i<=64; i++)
		{
			std::stringstream buf;
			std::string strPath = FeFileReg->GetDataPath();
			buf<<strPath<< "/texture/explosion/Explosion"<<std::setw(4)<< std::setfill('0') << i << ".png";
			std::string imageName = buf.str();

			osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imageName);
			if (image.valid())
			{
				m_rpExplosionSequence->addImage(image.get());
			}
		}

		unsigned int maxNum = m_rpExplosionSequence->getNumImageData();
		m_rpExplosionSequence->setLength(double(maxNum)*(1.0/30.0));
		m_rpExplosionSequence->setLoopingMode(osg::ImageStream::NO_LOOPING);

		m_rpFlameSequence = new osg::ImageSequence;
		for(int i = 45; i<=200; i+=10)
		{
			std::stringstream buf;
			buf<<FeFileReg->GetFullPath("model/fireimage/a")<< std::setw(4)<< std::setfill('0') << i << ".png";
			std::string imageName = buf.str();

			osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imageName);
			if (image.valid())
			{
				m_rpFlameSequence->addImage(image.get());
			}
		}

		maxNum = m_rpFlameSequence->getNumImageData();
		m_rpFlameSequence->setLength(double(maxNum)*(1.0/30.0));

	}

	CImageSequence::~CImageSequence()
	{

	}
}