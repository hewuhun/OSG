#include <FeEffects/explosion.h>
#include <FeUtils/CoordConverter.h>
#include <osg/Geode>

namespace FeEffect
{
	CExplosionElement::CExplosionElement(FeUtil::CRenderContext* pContext) 
		:osg::MatrixTransform()
		,FeUtil::CRenderContextObserver(pContext)
		,m_vecPosition(osg::Vec3d(0, 0, 0))
		,m_vecWind(osg::Vec3d(0, 0, 0))
		,m_dScale(100.0)
		,m_dIntensity(1.0)
		,m_rpExplosion(0L)
		,m_rpExplosionDebri(0L)
		,m_rpSmoke(0L)
		,m_rpFire(0L)
	{
		CreateExplosion();
		SetWind(m_vecWind);
		SetPosition(m_vecPosition);	
	}

	CExplosionElement::~CExplosionElement()
	{

	}


	osg::Group* CExplosionElement::CreateExplosion()
	{
		//默认构造
		osg::Vec3d position = osg::Vec3d(0, 0, 0);// 局部坐标，设置在中心点
		m_rpExplosion = new osgParticle::ExplosionEffect(position, m_dScale, m_dIntensity);
		m_rpExplosionDebri = new osgParticle::ExplosionDebrisEffect(position, m_dScale, m_dIntensity);
		m_rpSmoke = new osgParticle::SmokeEffect(position, m_dScale, m_dIntensity);
		m_rpFire = new osgParticle::FireEffect(position, m_dScale, m_dIntensity);

		osg::ref_ptr<osg::Group> effectsGroup = new osg::Group;
		effectsGroup->addChild(m_rpExplosion);
		effectsGroup->addChild(m_rpExplosionDebri);
		effectsGroup->addChild(m_rpSmoke);
		effectsGroup->addChild(m_rpFire);

		m_rpExplosion->setUseLocalParticleSystem(false);
		m_rpExplosionDebri->setUseLocalParticleSystem(false);
		m_rpSmoke->setUseLocalParticleSystem(false);
		m_rpFire->setUseLocalParticleSystem(false);

		osg::Geode* geode = new osg::Geode;
		geode->addDrawable(m_rpExplosion->getParticleSystem());
		geode->addDrawable(m_rpExplosionDebri->getParticleSystem());
		geode->addDrawable(m_rpSmoke->getParticleSystem());
		geode->addDrawable(m_rpFire->getParticleSystem());

		osg::ref_ptr<osg::Group> root = new osg::Group;

		root->addChild(effectsGroup);
		root->addChild(geode);

		addChild(root);

		return root.get();

	}


	void CExplosionElement::SetPosition( osg::Vec3d pos )
	{
		m_vecPosition = pos;
		osg::Matrix matrix;
		FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), pos, matrix);
		setMatrix(matrix);
	}

	void CExplosionElement::SetWind( osg::Vec3d wind )
	{
		m_vecWind = wind;
		m_rpExplosion->setWind(m_vecWind);
		m_rpExplosionDebri->setWind(m_vecWind);
		m_rpSmoke->setWind(m_vecWind);
		m_rpFire->setWind(m_vecWind);
	}

	void CExplosionElement::SetScale( double scale )
	{
		m_rpExplosion->setScale(scale);
		m_rpExplosionDebri->setScale(scale);
		m_rpSmoke->setScale(scale);
		m_rpFire->setScale(scale);
	}

	void CExplosionElement::SetIntensity( double intensity )
	{
		m_rpExplosion->setIntensity(intensity);
		m_rpExplosionDebri->setIntensity(intensity);
		m_rpSmoke->setIntensity(intensity);
		m_rpFire->setIntensity(intensity);
	}

}

