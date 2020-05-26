#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Billboard>
#include <osg/Depth>
#include <osgDB/ReadFile>
#include <iostream>
#include <iomanip>

#include <FeUtils/PathRegistry.h>
#include <FeEffects/FlameGeom.h>
#include <FeEffects/ExplosionEffect.h>

namespace FeEffect
{
	CFlameGeom::CFlameGeom(double dSize)
		:osg::MatrixTransform()
		,m_dSize(dSize)
	{
		Init();
	}

	CFlameGeom::~CFlameGeom()
	{

	}

	void CFlameGeom::Init()
	{
		m_rpImageSequence = new osg::ImageSequence;
		m_rpImageSequence->setLoopingMode(osg::ImageStream::NO_LOOPING);

		m_rpGeom = osg::createTexturedQuadGeometry(
			osg::Vec3(-m_dSize/2.0, 0.0f, -m_dSize/2.0),
			osg::Vec3(m_dSize, 0.0f, 0.0f),
			osg::Vec3(0.0f, 0.0f, m_dSize));

		osg::ref_ptr<osg::Texture2D> pTexture = new osg::Texture2D;
		pTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		pTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		pTexture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
		pTexture->setResizeNonPowerOfTwoHint(false);
		pTexture->setImage(m_rpImageSequence.get());

		osg::ref_ptr<osg::StateSet> pStateSet = new osg::StateSet;
		pStateSet->setTextureAttributeAndModes(0, pTexture, osg::StateAttribute::ON);
		pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		//osg::Depth* dp = new osg::Depth();
		//dp->setWriteMask(false);
		//this->getOrCreateStateSet()->setAttribute(dp, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		
		osg::ref_ptr<osg::Billboard> pBillboard = new osg::Billboard();
		pBillboard->setMode(osg::Billboard::POINT_ROT_EYE);
		pBillboard->addDrawable(m_rpGeom);
		pBillboard->setStateSet(pStateSet);

		addChild(pBillboard);
	}

	void CFlameGeom::Play()
	{
		m_rpImageSequence->rewind();
		m_rpImageSequence->play();
	}

	void CFlameGeom::Stop()
	{
		m_rpImageSequence->pause();
		m_rpImageSequence->rewind();
	}

	void CFlameGeom::AddImage(osg::Image* pImage)
	{
		if (m_rpImageSequence.valid())
		{
			m_rpImageSequence->addImage(pImage);

			unsigned nMaxNum = m_rpImageSequence->getNumImageData();
			m_rpImageSequence->setLength(double(nMaxNum)*(1.0 / 30.0));
		}
	}

	void CFlameGeom::AddImagePath(std::string strImagePath)
	{
		if (m_rpImageSequence.valid())
		{
			m_rpImageSequence->addImageFile(strImagePath);

			unsigned nMaxNum = m_rpImageSequence->getNumImageData();
			m_rpImageSequence->setLength(double(nMaxNum)*(1.0 / 30.0));
		}
	}

	void CFlameGeom::SetSize(double dSize)
	{
		m_rpGeom = osg::createTexturedQuadGeometry(
			osg::Vec3(-m_dSize / 2.0, 0.0f, -m_dSize / 2.0),
			osg::Vec3(m_dSize, 0.0f, 0.0f),
			osg::Vec3(0.0f, 0.0f, m_dSize));
	}

	void CFlameGeom::SetLoopMode(osg::ImageStream::LoopingMode mode)
	{
		if (m_rpImageSequence.valid())
		{
			m_rpImageSequence->setLoopingMode(mode);
		}
	}

	osg::ImageSequence* CFlameGeom::GetImageSequence()
	{
		return m_rpImageSequence.get();
	}

	void CFlameGeom::AddExplosionImage()
	{
		if (m_rpImageSequence.valid())
		{
			unsigned nMaxNum = CImageSequence::Instance()->GetExplosionSequence()->getNumImageData();

			for(unsigned int n =0 ;n<nMaxNum;n++)
			{
				m_rpImageSequence->setImage(n,CImageSequence::Instance()->GetExplosionSequence()->getImage(n));
			}

			m_rpImageSequence->setLength(double(nMaxNum)*(1.0 / 30.0));
		}
	}

}
