#include <osg/Math>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Image>
#include <osgText/Text>
#include <osg/Texture2D>
#include <osg/LineWidth>
#include <osgDB/ReadFile>
#include <osgDB/ConvertUTF>

#include <osgEarth/MapNode>

#include <FeUtils/UtilityGeom.h>
#include <FeUtils/CoordConverter.h>

#include <PathRegistry.h>
#include "osg/BlendFunc"
#include "osg/AlphaFunc"

namespace FeUtil
{
    osg::Geometry* createTexturedGeometry(
        const osg::Vec3d& corner,
        const osg::Vec3d& widthVec,
        const osg::Vec3d& heightVec, 
        const osg::Vec4d& vecColor,
        double l, 
        double b, 
        double r, 
        double t)
    {
        osg::Geometry* geom = new osg::Geometry;

        osg::Vec3dArray* coords = new osg::Vec3dArray(4);
        (*coords)[0] = corner+heightVec;
        (*coords)[1] = corner;
        (*coords)[2] = corner+widthVec;
        (*coords)[3] = corner+widthVec+heightVec;
        geom->setVertexArray(coords);

        osg::Vec2dArray* tcoords = new osg::Vec2dArray(4);
        (*tcoords)[0].set(l,t);
        (*tcoords)[1].set(l,b);
        (*tcoords)[2].set(r,b);
        (*tcoords)[3].set(r,t);
        geom->setTexCoordArray(0,tcoords);

        osg::Vec4dArray* colours = new osg::Vec4dArray(1);
        (*colours)[0] = vecColor;
        geom->setColorArray(colours);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);

        osg::Vec3dArray* normals = new osg::Vec3dArray(1);
        (*normals)[0] = widthVec^heightVec;
        (*normals)[0].normalize();
        geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

        geom->setUseVertexBufferObjects(true);

        return geom;
    }

    osg::Geode* CreateBasePlate(
        const std::string& strImagePath, 
        const osg::Vec3d& vecCorner, 
        double dWidthRate,
        double dHeightRate,
        const osg::Vec4d& vecColor)
    {
        osg::Geode* pGeode = new osg::Geode;

        osg::Geometry* pGeometry = createTexturedGeometry(
            vecCorner, 
            osg::Vec3d(dWidthRate, 0.0, 0.0),
            osg::Vec3d(0.0, dHeightRate, 0.0),
            vecColor,
            0.0, 
            0.0, 
            1.0, 
            1.0);

        pGeode->addDrawable(pGeometry);

        osg::Image* pImage = osgDB::readImageFile(strImagePath);
        if(pImage)
        {
            osg::Texture2D* pTexture = new osg::Texture2D;
            pTexture->setImage(pImage);

            pGeometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, pTexture);
        }

        pGeometry->getOrCreateStateSet()->setMode( 
            GL_BLEND, 
            osg::StateAttribute::ON | 
            osg::StateAttribute::OVERRIDE );
        pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING,
            osg::StateAttribute::OFF |
            osg::StateAttribute::PROTECTED|
            osg::StateAttribute::OVERRIDE);

        pGeometry->getOrCreateStateSet()->setMode(
            GL_DEPTH_TEST,
            osg::StateAttribute::ON |
            osg::StateAttribute::OVERRIDE);

        return pGeode;
    }

    osg::Geometry* CreateRectangle( 
        double dPosX,
        double dPosY,
        double dWidth, 
        double dHeight, 
        const osg::Vec4d& vecColor, 
		float fLineWidth /*= 1.0*/,
		bool bType /*= false*/ ,
		const osg::Vec4d& vecFillColor /*osg::Vec4d(1.0,1.0,0.0,0.5)*/)
    {
        osg::Geometry* pGeome = new osg::Geometry;

        osg::Vec3dArray* pVertex = new osg::Vec3dArray;
        pGeome->setVertexArray(pVertex);

        osg::Vec4dArray* pColor = new osg::Vec4dArray;
        pColor->push_back(vecColor);
        pGeome->setColorArray(pColor);
        pGeome->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);

        pVertex->push_back(osg::Vec3d(dPosX, dPosY, 0.0));
        pVertex->push_back(osg::Vec3d(dPosX + dWidth, dPosY, 0.0));
        pVertex->push_back(osg::Vec3d(dPosX + dWidth, dPosY + dHeight, 0.0));
        pVertex->push_back(osg::Vec3d(dPosX, dPosY + dHeight, 0.0));

		if(bType)
		{
			pGeome->addPrimitiveSet( 
				new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, pVertex->size()));		
		}
		else
		{
			pGeome->addPrimitiveSet(
				new osg::DrawArrays( osg::PrimitiveSet::LINE_LOOP, 0, pVertex->size()));
		}

        osg::LineWidth* pLineWidth = new osg::LineWidth(fLineWidth);

        osg::StateSet* pStateSet = pGeome->getOrCreateStateSet();
	
        pStateSet->setAttribute(pLineWidth, osg::StateAttribute::ON);
        pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
		pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		pStateSet->setRenderBinDetails(102, "RenderBin");


        return pGeome;
    }

    osg::Node* CreateParallelSphere( 
        double dHeight,
        double dOffsetAngle/* = 0.0*/, 
        double dLineWidth/* = 1.0*/, 
        bool bIsFill/* = true*/,
        osg::Vec4 colorSurface/* = osg::Vec4(1.0, 1.0, 0.0, 0.1)*/, 
        osg::Vec4 colorLine/* = osg::Vec4(1.0, 1.0, 0.0, 1.0)*/)
    {
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

        osg::ref_ptr<osg::Vec3Array> coordary = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array;
        
        //计算顶点
        coordary->push_back(osg::Vec3(0.0, 0.0, 0.0));
		double d2PI = 2.0 * osg::PI;
		double dDelta = d2PI / 360.0;
		for (double i = 0.0; i < d2PI; i += dDelta)
		{
			coordary->push_back(osg::Vec3d(dHeight * cos(i), dHeight * sin(i), 0.0));
		}

        geom->setVertexArray(coordary.get());

        //如果需要填充，则绘制填充区域
        if (bIsFill)
        {
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, coordary->size()));
            colorArray->push_back(colorSurface);
        }
        
        //绘制轨道线
        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 1, coordary->size()-1));
        colorArray->push_back(colorLine);

        //绑定颜色数组以及设置线宽
        geom->setColorArray(colorArray.get());
        geom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
        geom->getOrCreateStateSet()->setAttribute(new osg::LineWidth(dLineWidth));
        
        //透明度打开，灯光关闭
        geode->addDrawable(geom.get());
        geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
        geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
     //   geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		geode->getOrCreateStateSet()->setRenderBinDetails(100, "DepthSortedBin");
        osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc();  // blend func      
        blendFunc->setSource(osg::BlendFunc::SRC_ALPHA);         
        blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_SRC_ALPHA);          
        geode->getOrCreateStateSet()->setAttributeAndModes( blendFunc );  

        geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        mt->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(dOffsetAngle), osg::Vec3(1.0, 0.0, 0.0)));
        mt->addChild(geode.get());

        return mt.release();
    }

    extern FEUTIL_EXPORT osgText::Text* CreateImageAndText(
        osg::Vec3d corner,
        double dWidth,
        double dHeight,
        std::string strText, 
        std::string strImage )
    {
        //添加文字信息
        osg::ref_ptr<osgText::Font> pFont = osgText::readFontFile("simfang.ttf");
        osg::ref_ptr<osgText::Text> pText = new osgText::Text;
        pText->setText("");
        pText->setFont(pFont.release());
        pText->setFontResolution(64.0f,64.0f);
        pText->setColor(osg::Vec4d(1.0,1.0,0.0,1.0));
        pText->setCharacterSize(dWidth * 0.8);
        pText->setDataVariance(osg::Object::DYNAMIC);
        pText->setAlignment(osgText::Text::CENTER_BOTTOM);//设置对其方式
        pText->setPosition(osg::Vec3(corner.x(), corner.y(), 50.0));
        pText->setText(osgDB::convertStringFromCurrentCodePageToUTF8(strText), osgText::String::ENCODING_UTF8);
        pText->setAxisAlignment(osgText::TextBase::XZ_PLANE);

        return pText.release();
    }


	osg::Geometry* CreateTrilateral( 
		double dPosX,
		double dPosY,
		double dWidth, 
		double dHeight, 
		const osg::Vec4d& vecColor, 
		float fLineWidth /*= 1.0*/,
		bool bType /*= false*/)
	{
		osg::Geometry* pGeome = new osg::Geometry;

		osg::Vec3dArray* pVertex = new osg::Vec3dArray;
		pGeome->setVertexArray(pVertex);

		osg::Vec4dArray* pColor = new osg::Vec4dArray;
		pColor->push_back(vecColor);
		pGeome->setColorArray(pColor);
		pGeome->setColorBinding(osg::Geometry::BIND_OVERALL);

		pVertex->push_back(osg::Vec3d(dPosX, dPosY, 0.0));
		pVertex->push_back(osg::Vec3d(dPosX + dWidth, dPosY, 0.0));
		pVertex->push_back(osg::Vec3d(dPosX + dWidth/2.0, dPosY + dHeight, 0.0));

		if(bType)
		{
			pGeome->addPrimitiveSet(
				new osg::DrawArrays( osg::PrimitiveSet::TRIANGLE_STRIP, 0, pVertex->size()));
		}
		else
		{
			pGeome->addPrimitiveSet(
				new osg::DrawArrays( osg::PrimitiveSet::LINE_LOOP, 0, pVertex->size()));
		}
		

		osg::LineWidth* pLineWidth = new osg::LineWidth(fLineWidth);

		osg::StateSet* pStateSet = pGeome->getOrCreateStateSet();
		pStateSet->setAttribute(pLineWidth, osg::StateAttribute::ON);
		pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);

		return pGeome;
	}


	osg::Geometry* CreateCircle( 
		double dPosX,
		double dPosY,
		double dRadius,  
		const osg::Vec4d& vecColor, 
		float fLineWidth /*= 1.0*/,
		bool bType /*= false*/)
	{
		osg::Geometry* pGeome = new osg::Geometry;

		osg::Vec3dArray* pVertex = new osg::Vec3dArray;
		pGeome->setVertexArray(pVertex);

		osg::Vec4dArray* pColor = new osg::Vec4dArray;
		pColor->push_back(vecColor);
		pGeome->setColorArray(pColor);
		pGeome->setColorBinding(osg::Geometry::BIND_OVERALL);

		osg::Vec3 center = osg::Vec3(dPosX,dPosY,0.0);
		double alp;


		for (double m=0; m<=360; m+=10)
		{
			alp = osg::DegreesToRadians(m);
			pVertex->push_back(center+osg::Vec3(dRadius*sinf(alp), dRadius*cosf(alp), 0.0));
		}

		if(bType)
		{
			pGeome->addPrimitiveSet(
				new osg::DrawArrays( osg::PrimitiveSet::TRIANGLE_FAN, 0, pVertex->size()));
		}
		else
		{
			pGeome->addPrimitiveSet(
				new osg::DrawArrays( osg::PrimitiveSet::LINE_LOOP, 0, pVertex->size()));
		}
		

		osg::LineWidth* pLineWidth = new osg::LineWidth(fLineWidth);

		osg::StateSet* pStateSet = pGeome->getOrCreateStateSet();
		pStateSet->setAttribute(pLineWidth, osg::StateAttribute::ON);
		pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
	
		return pGeome;
	}



osg::Drawable* CreateImageSquare(const osg::Vec3& corner, const osg::Vec3& width, const osg::Vec3& height, osg::Image* image)
{
	// set up the Geometry.
	osg::Geometry* geom = new osg::Geometry;

	osg::Vec3Array* coords = new osg::Vec3Array(4);
	(*coords)[0] = corner;
	(*coords)[1] = corner + width;
	(*coords)[2] = corner + width + height;
	(*coords)[3] = corner + height;


	geom->setVertexArray(coords);

	osg::Vec3Array* norms = new osg::Vec3Array(1);
	(*norms)[0] = width^height;
	(*norms)[0].normalize();

	geom->setNormalArray(norms, osg::Array::BIND_OVERALL);

	osg::Vec2Array* tcoords = new osg::Vec2Array(4);
	(*tcoords)[0].set(0.0f, 0.0f);
	(*tcoords)[1].set(1.0f, 0.0f);
	(*tcoords)[2].set(1.0f, 1.0f);
	(*tcoords)[3].set(0.0f, 1.0f);
	geom->setTexCoordArray(0, tcoords);

	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	if (image)
	{
		osg::StateSet* stateset = new osg::StateSet;
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setImage(image);
		stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

		osg::ref_ptr<osg::TexEnv> texEnv = new osg::TexEnv();
		texEnv->setMode(osg::TexEnv::REPLACE);
		stateset->setTextureAttribute(0,texEnv.get());  

		geom->setStateSet(stateset);
	}

	return geom;
}

}