#include <osg/PolygonMode>
#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/MatrixTransform>
#include <osg/AnimationPath>
#include <osg/LineWidth>
#include <osg/Image>

#include <osgDB/ReadFile>
#include <osg/Depth>

#include <iostream>
#include <FeEffects/HighlightEffect.h>

namespace FeEffect
{
	CHighlightEffect::CHighlightEffect(double radius,  osg::Vec4d color, bool grid, float radio)
		:m_vecColor(color)
		,m_dRadius(radius)
		,m_blGrid(grid)
	{

		CreateEffect();
	}


	CHighlightEffect::~CHighlightEffect(void)
	{
	}

	void CHighlightEffect::updateRadius( double radius )
	{
		m_dRadius  = radius;
		m_pSphere->setRadius(radius);
	}

	void CHighlightEffect::SetColor(osg::Vec4d &vecColor)
	{
		m_vecColor = vecColor;
		m_pSphereDrawable->setColor(m_vecColor);
	}

	osg::Geometry* CHighlightEffect::genCircle( double radius, osg::Vec4d color /*= osg::Vec4d(1.0,1.0,1.0,1.0)*/ )
	{
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		osg::Vec3dArray* vertexes = new osg::Vec3dArray;
		for (int i=0; i<36; i++)
		{
			vertexes->push_back(osg::Vec3d(cos(osg::DegreesToRadians(i*10.0)) * radius, radius*sin(osg::DegreesToRadians(i*10.0)), 0));
		}
		geom->setVertexArray(vertexes);
		osg::Vec4dArray* colorArray = new osg::Vec4dArray;
		colorArray->push_back(color);
		geom->setColorArray(colorArray);
		geom->setColorBinding(osg::Geometry::BIND_OVERALL);
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, vertexes->size()));

		return  geom.release();
	}

	bool CHighlightEffect::CreateEffect()
	{
		osg::Depth* dp = new osg::Depth();
		dp->setWriteMask(false);
		this->getOrCreateStateSet()->setAttribute(dp, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		osg::ref_ptr<osg::Geode> gnode = new osg::Geode;

		osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
		hints->setDetailRatio(0.6f);

		m_pSphere = new osg::Sphere(osg::Vec3d(0.0,0.0,0.0), m_dRadius);

		m_pSphereDrawable = new osg::ShapeDrawable(m_pSphere, hints.get());
		m_pSphereDrawable->setColor(m_vecColor);
		m_pSphereDrawable->dirtyBound();
		m_pSphereDrawable->dirtyDisplayList();
		gnode->addDrawable(m_pSphereDrawable);

		this->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
		this->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		if(m_blGrid)
		{
			osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;
			polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
			gnode->getOrCreateStateSet()->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
			osg::ref_ptr<osg::LineWidth> rpLineWidth = new osg::LineWidth(0.5);
			gnode->getOrCreateStateSet()->setAttributeAndModes(rpLineWidth,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		}
		
		gnode->addUpdateCallback(new CHighlightCallback(m_pSphereDrawable));

		osg::ref_ptr<osg::MatrixTransform> rpTransMt = new osg::MatrixTransform();
		rpTransMt->setMatrix(osg::Matrix::rotate(osg::PI_2,osg::X_AXIS));
		rpTransMt->addChild(gnode.get());
		addChild(rpTransMt.get());

		return true;
	}

}

namespace FeEffect
{

	CHighlightCallback::CHighlightCallback( osg::ShapeDrawable *pGeom)
		:osg::NodeCallback()
		,m_rpGeom(pGeom)
	{

	}

	CHighlightCallback::~CHighlightCallback()
	{

	}

	void CHighlightCallback::operator()( osg::Node *pNode,osg::NodeVisitor *pNv )
	{
		static double dTime = pNv->getFrameStamp()->getSimulationTime();
		double dTimeEnd = pNv->getFrameStamp()->getSimulationTime();

		if(dTimeEnd - dTime < 0.1)
		{
			osg::NodeCallback::traverse(pNode,pNv);
			return;
		}
		dTime = dTimeEnd;
		if(!m_rpGeom)
		{
			osg::NodeCallback::traverse(pNode,pNv);
			return;
		}

		osg::Vec4d vecColor = m_rpGeom->getColor();
		static double dUNit = 0.05;
		
		if(vecColor.a() >= 1.0||vecColor.a() <= 0.0)
		{
			dUNit = -dUNit;
		}
		vecColor.a() += dUNit;

		m_rpGeom->setColor(vecColor);

		m_rpGeom->dirtyDisplayList();

		osg::NodeCallback::traverse(pNode,pNv);
	}

}
