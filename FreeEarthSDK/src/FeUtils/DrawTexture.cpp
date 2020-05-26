#include <FeUtils/DrawTexture.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Array>

#include <osg/Image>
#include <osgDB/ReadFile>
#include <PathRegistry.h>

#include <osg/BlendColor>
#include <osg/AutoTransform>
#include <osg/Billboard>

#include <PathRegistry.h>

namespace FeUtil
{
	CDrawTexture::CDrawTexture(std::string strPath,double dSize)
		:m_Size(dSize)
		,m_strPath(strPath)
		,m_pImage(NULL)
	{
		Create();
	}

	CDrawTexture::CDrawTexture( osg::Image *pImage,double dSize )
		:m_Size(dSize)
		,m_strPath("")
		,m_pImage(pImage)
	{
		Create();
	}


	CDrawTexture::~CDrawTexture()
	{

	}

	void CDrawTexture::Create()
	{
		setName("CDrawTexture");
		osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
		osg::ref_ptr<osg::Geometry> pGeom = new osg::Geometry;
		
		osg::ref_ptr<osg::Vec3dArray> vertexArray = new osg::Vec3dArray;
		osg::ref_ptr<osg::Vec4dArray> colorArray = new osg::Vec4dArray;

		vertexArray->push_back(osg::Vec3d(-m_Size,-m_Size,0.0));
		vertexArray->push_back(osg::Vec3d(m_Size,-m_Size,0.0));
		vertexArray->push_back(osg::Vec3d(m_Size,m_Size,0.0));
		vertexArray->push_back(osg::Vec3d(-m_Size,m_Size,0.0));

		colorArray->push_back(osg::Vec4d(1.0,1.0,1.0,1.0));

		osg::ref_ptr<osg::Vec2Array> coord=new osg::Vec2Array;  
		coord->push_back(osg::Vec2(0.0,0.0));  
		coord->push_back(osg::Vec2(1.0,0.0));  
		coord->push_back(osg::Vec2(1.0,1.0));  
		coord->push_back(osg::Vec2(0.0,1.0));  


		pGeom->setVertexArray(vertexArray.get());
		pGeom->setColorArray(colorArray.get());
		pGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

		pGeom->setTexCoordArray(0,coord.get());
		pGeom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

		pGeode->addDrawable(pGeom.get());

		m_pTex2D = new osg::Texture2D;
		if(!m_pImage)
		{
			 m_pImage = CReadFileRegistry::Instance()->ReadImageFileInstance(m_strPath);//osgDB::readImageFile(m_strPath);
		}

		if(m_pImage && m_pTex2D.valid())
		{
			m_pTex2D->setImage(0,m_pImage);
		}

		osg::ref_ptr<osg::Geometry> rpGeom = osg::createTexturedQuadGeometry(
			osg::Vec3(-m_Size/2.0, 0.0f, -m_Size/2.0),
			osg::Vec3(m_Size, 0.0f, 0.0f),
			osg::Vec3(0.0f, 0.0f, m_Size));
		osg::StateSet *pState = rpGeom->getOrCreateStateSet();

		pState->setTextureAttributeAndModes(0,m_pTex2D.get(),osg::StateAttribute::ON);
		pState->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		pState->setMode(GL_BLEND,osg::StateAttribute::ON);
		pState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		//pState->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);

		osg::ref_ptr<osg::Billboard> center = new osg::Billboard();
		center->setMode(osg::Billboard::POINT_ROT_EYE);
		center->addDrawable(rpGeom);
		//center->setStateSet(pState);

		osg::ref_ptr<osg::AutoTransform> pAutoMT = new osg::AutoTransform;
		pAutoMT->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
		pAutoMT->setAutoScaleToScreen(true);
		//pAutoMT->setMinimumScale(10.0);
		//pAutoMT->setMaximumScale(8);
		pAutoMT->addChild(center.get());

		addChild(pAutoMT.get());

	}

	void CDrawTexture::SetImage( osg::Image *pImage )
	{
		if(m_pTex2D.valid() && pImage)
		{
			m_pImage = pImage;
			m_pTex2D->setImage(0,pImage);
		}
	}

	osg::Image * CDrawTexture::GetImage()
	{
		return m_pImage;
	}

}

