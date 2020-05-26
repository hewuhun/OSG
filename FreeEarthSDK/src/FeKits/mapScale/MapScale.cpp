#include <iostream>
#include <sstream>
#include <iomanip>
#include <osgGA/EventVisitor>
#include <osg/LineWidth>
#include <osgDB/ConvertUTF>
//#include <osgFx/Outline>

#include <FeKits/mapScale/MapScale.h>
#include <FeUtils/MathUtil.h>

#include <FeUtils/CoordConverter.h>
#include "FeUtils/PathRegistry.h"

namespace FeKit
{

	const double M2KM = 1000.0; //米和千米的转化

	CMapScale::CMapScale(
		FeUtil::CRenderContext* pContext,
		int nXOffset, 
		int nYOffset, 
		int nWidth, 
		int nHeight, 
		CPlaceStrategy::KIT_PLACE ePlace)
		:CScreenKits(pContext, nXOffset, nYOffset, nWidth, nHeight, ePlace)
		,m_opRatioMS(NULL)
		,m_opDigtalMS(NULL)
		,m_dDistance(0.0)
		,m_vecStartPosRate(0.0, 0.0, -1.0)
		,m_vecEndPosRate(0.0, 0.0, -1.0)
		,m_bClear(false)
		,m_rpMapScaleNode(NULL)
	{
		setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal() + 1);
		m_rpMapScaleNode = CreateMSUI();

		//osg::ref_ptr<osgFX::Outline> ol = new osgFX::Outline;
		//ol->addChild(m_rpMapScaleNode.get());
		//ol->setColor(osg::Vec4(0,0,0,1));
		//ol->setWidth(1.0);
		//addChild(ol);

		addChild(m_rpMapScaleNode);
	}


	CMapScale::~CMapScale(void)
	{
	}


	osg::Geode* CMapScale::CreateMSUI()
	{
		double dTemp = m_dRightRate - m_dLeftRate;

		double dStartX = m_dLeftRate  + dTemp * 0.1;
		double dStartY = m_dBottomRate * 0.5;

		double dLongY = m_dTopRate * 0.25;
		double dShortY = 0.0;

		double dDeltaX = dTemp * 0.8 / 4.0;

		osg::Geode* pGeode = new osg::Geode;
		//图形比例尺
		osg::Geometry* pGeome = new osg::Geometry;
		osg::Vec3dArray* pVertex = new osg::Vec3dArray;

		pVertex->push_back(osg::Vec3d(dStartX, dLongY, m_dDepth)); //0
		pVertex->push_back(osg::Vec3d(dStartX, dStartY, m_dDepth)); //1

		pVertex->push_back(osg::Vec3d(dStartX + dDeltaX, dShortY, m_dDepth)); //2
		pVertex->push_back(osg::Vec3d(dStartX + dDeltaX, dStartY, m_dDepth)); //3

		pVertex->push_back(osg::Vec3d(dStartX + 2 * dDeltaX, dLongY, m_dDepth)); //4
		pVertex->push_back(osg::Vec3d(dStartX + 2 * dDeltaX, dStartY, m_dDepth)); //5

		pVertex->push_back(osg::Vec3d(dStartX + 3 * dDeltaX, dShortY, m_dDepth)); //6
		pVertex->push_back(osg::Vec3d(dStartX + 3 * dDeltaX, dStartY, m_dDepth)); //7

		pVertex->push_back(osg::Vec3d(dStartX + 4 * dDeltaX, dLongY, m_dDepth)); //8
		pVertex->push_back(osg::Vec3d(dStartX + 4 * dDeltaX, dStartY, m_dDepth)); //9

		pVertex->push_back(osg::Vec3d(dStartX, dStartY, m_dDepth)); //10
		pVertex->push_back(osg::Vec3d(dStartX + 4 * dDeltaX, dStartY, m_dDepth)); //9

		m_vecEndPosRate.set(dStartX + 4 * dDeltaX, dStartY, m_dDepth);
		m_vecStartPosRate.set(dStartX + 3 * dDeltaX, dStartY, m_dDepth);
		m_vecDigtalRate.set(dStartX + 4 * dDeltaX*0.9, 2.0*dLongY, m_dDepth);

		pGeome->setVertexArray(pVertex);

		osg::Vec4dArray* pColor = new osg::Vec4dArray;
		osg::Vec4d color(0.8, 0.8, 0.8, 0.8);
		pColor->push_back(color);
		pGeome->setColorArray(pColor);
		pGeome->setColorBinding(osg::Geometry::BIND_OVERALL);

		pGeome->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, pVertex->size()));
		pGeome->getOrCreateStateSet()->setAttribute(new osg::LineWidth(2.0), osg::StateAttribute::PROTECTED);
		//	pGeode->addDrawable(pGeome);

		//数字比例尺
		m_opDigtalMS = new osgText::Text;
		osg::ref_ptr<osgText::Font> pFont = osgText::readFontFile("fonts/simhei.ttf");
		m_opDigtalMS->setFont(pFont.get());
		//	m_opDigtalMS->setFontResolution(128.0,128.0);
		m_opDigtalMS->setColor(osg::Vec4d(1,1,1,1.0));
		m_opDigtalMS->setCharacterSize(m_dRightRate * 0.08);
		m_opDigtalMS->setDataVariance(osg::Object::DYNAMIC);
		m_opDigtalMS->setAlignment(osgText::Text::CENTER_CENTER);//设置对其方式
		m_opDigtalMS->setPosition(m_vecDigtalRate);
		m_opDigtalMS->setBackdropColor(osg::Vec4(0,0,0,1));
		m_opDigtalMS->setBackdropImplementation(osgText::Text::STENCIL_BUFFER);
		m_opDigtalMS->setBackdropType(osgText::Text::OUTLINE);
		m_opDigtalMS->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
		m_opDigtalMS->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		pGeode->addDrawable(m_opDigtalMS.get());

		//图片比例尺
		osg::Geometry* geom = new osg::Geometry;
		osg::Vec3dArray* vertices = new osg::Vec3dArray;
		osg::Vec4dArray* colors = new osg::Vec4dArray;
		vertices->push_back(osg::Vec3d(dStartX, dStartY, m_dDepth)); 
		vertices->push_back(osg::Vec3d(dStartX, dLongY, m_dDepth)); 
		vertices->push_back(osg::Vec3d(dStartX + 4 * dDeltaX, dLongY, m_dDepth)); 
		vertices->push_back(osg::Vec3d(dStartX + 4 * dDeltaX, dStartY, m_dDepth)); 
		geom->setVertexArray(vertices);
		colors->push_back(osg::Vec4d(1,1,1,1));
		geom->setColorArray(colors);
		geom->setColorBinding(osg::Geometry::BIND_OVERALL);
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertices->size()));
		osg::Vec2dArray* tcoords = new osg::Vec2dArray(4);
		(*tcoords)[3].set(1.0f,0.0f);
		(*tcoords)[2].set(1.0f,1.0f);
		(*tcoords)[1].set(0.0f,1.0f);
		(*tcoords)[0].set(0.0f,0.0f);
		geom->setTexCoordArray(0,tcoords);
		osg::Texture2D* tex = new osg::Texture2D;
		tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		tex->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
		tex->setResizeNonPowerOfTwoHint(false);
                tex->setImage(osgDB::readImageFile(FeFileReg->GetFullPath("texture/mapScale/mapScale.png")));
		geom->getOrCreateStateSet()->setTextureAttributeAndModes(0,tex,osg::StateAttribute::ON);
		pGeode->addDrawable(geom);

		osg::StateSet* pStateSet = pGeode->getOrCreateStateSet();

		pStateSet->setMode( 
			GL_BLEND, 
			osg::StateAttribute::ON | 
			osg::StateAttribute::OVERRIDE );
		pStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		pStateSet->setMode(
			GL_LIGHTING,
			osg::StateAttribute::OFF |
			osg::StateAttribute::PROTECTED|
			osg::StateAttribute::OVERRIDE);

		pStateSet->setMode(
			GL_DEPTH_TEST,
			osg::StateAttribute::ON |
			osg::StateAttribute::OVERRIDE);

		return pGeode;

	}

	void CMapScale::UpdateScale()
	{
		if(m_bShow) 
		{
			if (m_opDigtalMS.valid())
			{
				double dStartScreenX = m_nX + m_nWidth * 0.7;
				double dStartScreenY = m_nY + m_nHeight * 0.5;

				double dEndScreenX = m_nX + m_nWidth * 0.9;
				double dEndScreenY = dStartScreenY;

				osg::Vec3d dStartLLH(0.0, 0.0, 0.0);
				osg::Vec3d dEndLLH(0.0, 0.0, 0.0);

				///两个点都和地球有交点才计算距离
				if(FeUtil::ScreenXY2RadiaLLH(m_opRenderContext.get(), dStartScreenX, dStartScreenY, dStartLLH) && FeUtil::ScreenXY2RadiaLLH(m_opRenderContext.get(), dEndScreenX, dEndScreenY, dEndLLH))
				{
					double dDistance = FeUtil::GetGeoDistance(m_opRenderContext.get(), dStartLLH.x(), dStartLLH.y(), dEndLLH.x(), dEndLLH.y());
					if (dDistance < 1.0)
					{
						dDistance = 1.0;
					}

					if(!FeMath::Equal(m_dDistance, dDistance))
					{
						m_dDistance = dDistance;

						std::stringstream stdStream;
						if(dDistance >= M2KM) //转化为千米
						{
							dDistance /= M2KM;
                            stdStream<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<dDistance<<"Km";
						}
						else
						{
                             stdStream<<std::setiosflags(std::ios::fixed)<<std::setprecision(0)<<dDistance<<"m";
						}

						std::string strInfo = osgDB::convertStringFromCurrentCodePageToUTF8(stdStream.str());
						m_opDigtalMS->setText(strInfo, osgText::String::ENCODING_UTF8);
					}

					if(m_bClear)
					{
						if(m_rpMapScaleNode.valid())
						{
							addChild(m_rpMapScaleNode.get());
							m_bClear = false;
						}
					}
				}
				else
				{
					if(m_opDigtalMS.valid())
					{
						m_opDigtalMS->setText("");
					}
					if(m_rpMapScaleNode.valid())
					{
						removeChild(m_rpMapScaleNode.get());
						m_bClear = true;
					}
				}
			}
		}
	}

	void CMapScale::traverse( osg::NodeVisitor& nv )
	{
		if (nv.getVisitorType()==osg::NodeVisitor::EVENT_VISITOR)
		{
			UpdateScale();
		}

		CScreenKits::traverse(nv);
	}

	double CMapScale::GetDistance()
	{
		return m_dDistance;
	}

}

