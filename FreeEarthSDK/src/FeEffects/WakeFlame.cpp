#include <FeEffects/WakeFlame.h>
#include <FeEffects/ExplosionEffect.h>
#include <FeUtils/PathRegistry.h>

#include <osg/PrimitiveSet>
#include <osg/AnimationPath>
#include <osg/LineWidth>
#include <osgText/Text>
#include <sstream>
#include <osg/ImageSequence>
#include <osg/Texture2D>
#include <osg/Billboard>
#include <osg/Depth>
#include <osgDB/ReadFile>

#include <iostream>
#include <iomanip>

using namespace osg;

osg::Geode* GetGeode(osg::Node* node)
{
	if(node->asGeode())
	{
		return node->asGeode();
	}

	osg::Group* root = node->asGroup();


	if(NULL != root)
	{
		for(unsigned int i = 0; i<root->getNumChildren(); i++)
		{
			if(NULL != GetGeode(root->getChild(i)))
			{
				return GetGeode(root->getChild(i));
			}
		}
	}

	return NULL;
}

namespace FeEffect
{
	CGeoTile::CGeoTile()
		:CFreeEffect()
		,m_rpImageSequence(NULL)
		,m_pTransMT(NULL)
		,m_pScaleMT(NULL)
		,m_pRotateMT(NULL)
	{
		CreateEffect();

	}

	CGeoTile::~CGeoTile()
	{

	}


	bool CGeoTile::StartEffect()
	{
		m_rpImageSequence->rewind();
		m_rpImageSequence->play();

		return true;
	}

	bool CGeoTile::CreateEffect()
	{

		osg::Node* node = osgDB::readNodeFile(FeFileReg->GetFullPath("model/WakeFlame/wei2.ive"));
		m_pTransMT =   new  osg::MatrixTransform;
		m_pScaleMT =   new  osg::MatrixTransform;
		m_pRotateMT =  new  osg::MatrixTransform;

		m_pTransMT->addChild(m_pRotateMT.get());
		m_pRotateMT->addChild(m_pScaleMT.get());
		m_pScaleMT->addChild(node);

		m_pTransMT->setMatrix(osg::Matrix::translate(0.0,-1.8,0.0));
		m_pRotateMT->setMatrix(osg::Matrix::rotate(-osg::PI_2,osg::X_AXIS));
		m_pScaleMT->setMatrix(osg::Matrix::scale(0.001,0.001,0.001));

		addChild(m_pTransMT.get());


		m_rpImageSequence = new osg::ImageSequence;
		unsigned nMaxNum = CImageSequence::Instance()->GetFlameSequence()->getNumImageData();
		for(unsigned int n =0 ;n<nMaxNum;n++)
		{
			m_rpImageSequence->setImage(n,CImageSequence::Instance()->GetFlameSequence()->getImage(n));
		}

		m_rpImageSequence->setLength(double(nMaxNum)*(1.0 / 30.0));

		m_rpImageSequence->play();

		osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
		texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::NEAREST);
		texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::NEAREST);
		texture->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);
		texture->setResizeNonPowerOfTwoHint(false);
		texture->setImage(m_rpImageSequence.get());

		osg::Geode* gnode = GetGeode(node);
		osg::StateSet* stateset2 = gnode->getDrawable(0)->getOrCreateStateSet();
		stateset2->setTextureAttributeAndModes(0,texture.get(),osg::StateAttribute::ON);
		stateset2->setMode(GL_BLEND, osg::StateAttribute::ON );
		stateset2->setMode(GL_LIGHTING, osg::StateAttribute::OFF );
		stateset2->setRenderingHint(osg::StateSet::OPAQUE_BIN);

		return true;
	}

	bool CGeoTile::ClearEffect()
	{
		removeChildren(0,this->getNumChildren());
		return false;
	}

}