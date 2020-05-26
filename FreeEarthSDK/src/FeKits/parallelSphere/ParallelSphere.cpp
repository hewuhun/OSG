#include <osg/Group>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/LineWidth>

#include <FeKits/parallelSphere/ParallelSphere.h>
#include <FeUtils/UtilityGeom.h>
#include "osg/PolygonOffset"
#include "osg/BlendFunc"
#include "osg/AlphaFunc"
#include "osg/Depth"

namespace FeKit
{
	const double D_EQUATOR_ECLIPTIC_RADIA = 23.4333; //黄赤交角

    CCelestialReference::CCelestialReference(FeUtil::CRenderContext* pContext )
		:FeKit::CKitsCtrl(pContext)
		,osg::Group()
		,m_opAxis(NULL)
		,m_opEquator(NULL)
		,m_opEcliptic(NULL)
		,m_dRadius(osg::WGS_84_RADIUS_EQUATOR)
    	,m_dLineWidth(2.0)
        ,m_colorFill(osg::Vec4(1.0, 1.0, 0.0, 0.1))
    	,m_colorLine(osg::Vec4(1.0, 0.0, 0.0, 0.9))
	{
		CreateUnit();
	}

	CCelestialReference::CCelestialReference( FeUtil::CRenderContext* pContext, 
		double dRadius, 
		double dLineWidth /*= 1.0*/, 
		osg::Vec4 colorFill /*= osg::Vec4(1.0, 1.0, 0.0, 0.1)*/, 
		osg::Vec4 colorLine /*= osg::Vec4(1.0, 1.0, 0.0, 0.9)*/ )
		:FeKit::CKitsCtrl(pContext)
		,osg::Group()
		,m_opAxis(NULL)
		,m_opEquator(NULL)
		,m_opEcliptic(NULL)
		,m_dRadius(dRadius)
		,m_dLineWidth(dLineWidth)
		,m_colorFill(colorFill)
		,m_colorLine(colorLine)
	{
		CreateUnit();
	}

    CCelestialReference::~CCelestialReference( void )
    {

    }

    void CCelestialReference::Show()
    {
        setNodeMask(0xFFFFFFFF);
        m_bShow = true;
    }

    void CCelestialReference::Hide()
    {
        setNodeMask(0x00000000);
        m_bShow = false;
    }

	void CCelestialReference::CreateUnit()
	{
		if( m_bShow)
		{
			Show();
		}
		else
		{
			Hide();
		}

		m_opAxis = CreateAxis();
		if (m_opAxis.valid())
		{
			addChild(m_opAxis.get());
		}

		m_opEquator = CreateEquator();
		if (m_opEquator.valid())
		{
			addChild(m_opEquator.get());
            //m_opEquator->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(-637813, 10), osg::StateAttribute::ON);
		}

		m_opEcliptic = CreateEcliptic();
		if (m_opEcliptic.valid())
		{
           //m_opEcliptic->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(63783, 1), osg::StateAttribute::ON);
			addChild(m_opEcliptic.get());
		}
     //   getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
     //   getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1, 638), osg::StateAttribute::ON);

        osg::ref_ptr<osg::Depth> dp = new osg::Depth;
        dp->setFunction(osg::Depth::LESS);
        dp->setWriteMask(false);
        getOrCreateStateSet()->setAttributeAndModes( dp );  
	}

	void CCelestialReference::SetEquatorVisible( bool bVisible )
	{
		if (m_opEquator.valid())
		{
			if (bVisible)
			{
				m_opEquator->setNodeMask(0xFFFFFFFF);
			}
			else
			{
				m_opEquator->setNodeMask(0x00000000);
			}
		}
	}

	void CCelestialReference::SetEclipticVisible( bool bVisible )
	{
		if(m_opEcliptic.valid())
		{
			if(bVisible)
			{
				m_opEcliptic->setNodeMask(0xFFFFFFFF);
			}
			else
			{
				m_opEcliptic->setNodeMask(0x00000000);
			}
		}
	}

	void CCelestialReference::SetAxisVisible( bool bVisible )
	{
		if (m_opAxis.valid())
		{
			if (bVisible)
			{
				m_opAxis->setNodeMask(0xFFFFFFFF);
			}
			else
			{
				m_opAxis->setNodeMask(0x00000000);
			}
		}
	}

	osg::Node* CCelestialReference::CreateEquator()
	{
		return FeUtil::CreateParallelSphere(m_dRadius * 2.5, 0.0, 1.0, true, m_colorFill, m_colorLine);
	}

	osg::Node* CCelestialReference::CreateEcliptic()
	{
		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

		osg::ref_ptr<osg::Vec3Array> coordary = new osg::Vec3Array;
		osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array;

		//计算顶点
		double dPosX = -m_dRadius*3;
		double dPosY =-m_dRadius*2;
		double dWidth = m_dRadius*6;
		double dHeight = m_dRadius*4;
		coordary->push_back(osg::Vec3d(dPosX, dPosY, 0.0));
		coordary->push_back(osg::Vec3d(dPosX + dWidth, dPosY, 0.0));
		coordary->push_back(osg::Vec3d(dPosX + dWidth, dPosY + dHeight, 0.0));
		coordary->push_back(osg::Vec3d(dPosX, dPosY + dHeight, 0.0));

		geom->setVertexArray(coordary.get());

		//如果需要填充，则绘制填充区域
		if (true)
		{
			geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, coordary->size()));
			colorArray->push_back(osg::Vec4d(1.0,1.0,0.0,0.2));
		}

		//绘制轨道线
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, coordary->size()));
		colorArray->push_back(osg::Vec4d(1.0,1.0,0.0,0.8));

		//绑定颜色数组以及设置线宽
		geom->setColorArray(colorArray.get());
		geom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
		geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		geom->getOrCreateStateSet()->setAttribute(new osg::LineWidth(1));

		geode->addDrawable(geom);

		geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
        geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
        osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc();  // blend func    
        blendFunc->setSource(osg::BlendFunc::SRC_ALPHA);         
        blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_SRC_ALPHA);          
        geode->getOrCreateStateSet()->setAttributeAndModes( blendFunc );  

        geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN); // geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		geode->getOrCreateStateSet()->setRenderBinDetails(200, "DepthSortedBin");
		osg::MatrixTransform* pMT = new osg::MatrixTransform;
		pMT->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(D_EQUATOR_ECLIPTIC_RADIA), osg::Y_AXIS));
		pMT->addChild(geode);
		return pMT;
	}

	osg::Node* CCelestialReference::CreateAxis()
	{
		double dDeltaHeight = osg::WGS_84_RADIUS_EQUATOR * 2; //顶点到地表的高度
		osg::ref_ptr<osg::Group> rpAxis = new osg::Group();
		osg::ref_ptr<osg::Geode> rfGeode = new osg::Geode;
		rpAxis->addChild(rfGeode.get());
		osg::ref_ptr<osg::Geometry> rfGeom = new osg::Geometry;

		osg::ref_ptr<osg::Vec3dArray> vecCorrdary = new osg::Vec3dArray;
		osg::ref_ptr<osg::Vec4dArray> vecColor = new osg::Vec4dArray;

		vecCorrdary->push_back(osg::Vec3d(0.0,0.0,m_dRadius*3));
		vecCorrdary->push_back(osg::Vec3d(0.0,0.0,-(m_dRadius*4)));

		//设置顶点关联
		rfGeom->setVertexArray(vecCorrdary);
		rfGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vecCorrdary->size()));

		//设定颜色及线宽
		vecColor->push_back(osg::Vec4d(1.0,1.0,1.0,0.9));
		rfGeom->setColorArray(vecColor);
		rfGeom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
		rfGeom->getOrCreateStateSet()->setAttribute(new osg::LineWidth(1.5),osg::StateAttribute::ON);

		//将gemo加入geode,透明度打开，灯光关闭
		rfGeode->addDrawable(rfGeom);
		rfGeode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		rfGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
		//rfGeode->getOrCreateStateSet()->setRenderBinDetails(100, "RenderBin");

        osg::ref_ptr<osg::Depth> dp = new osg::Depth;
        dp->setFunction(osg::Depth::LESS);
        dp->setWriteMask(true);
        rfGeode->getOrCreateStateSet()->setAttributeAndModes( dp, osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );  

		return rpAxis.release();
	}

}

