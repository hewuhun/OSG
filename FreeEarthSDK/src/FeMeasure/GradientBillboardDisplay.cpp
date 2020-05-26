#include <FeMeasure/GradientBillBoardDisplay.h>

#include <osg/Depth>
#include <osg/LineStipple>
#include <osgDB/ConvertUTF>
#include <osgEarthAnnotation/AnnotationUtils>
#include <FeUtils/PathRegistry.h>

namespace FeMeasure
{
		osg::Vec4d g_v4dLineColor(1.0, 0.5, 0.0, 1.0);
		osg::Vec4d g_v4dBackColor(0.7, 0.7, 0.7, 1.0);
		osg::Vec4d g_v4dTextColor(0.0, 0.0, 0.0, 1.0);
		double g_dMargin = 2.0;

	void CGradientBillBoardDisplay::CreateElement()
	{
		m_pObjectGroup = new osg::Group;
		getAttachPoint()->addChild(m_pObjectGroup);

		m_pFixedLineGeometry = new osg::Geometry;
		m_pFixedLineGeode = new osg::Geode;

		m_pFixedLineGeode->setComputeBoundingSphereCallback(new osgEarth::Annotation::ControlPointCallback());
		m_pObjectGroup->addChild(m_pFixedLineGeode);
		osg::StateSet* stateSet = m_pFixedLineGeode->getOrCreateStateSet();
		stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), 1);

		m_pLineWidth = new osg::LineWidth(float(1.0));
		m_pFixedLineGeode->addDrawable(m_pFixedLineGeometry);

		m_pFixedLineGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP, 0, 4));    // 
		m_pFixedLineGeometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 4, 4));    // 

		m_pFixedLineGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		m_pFixedLineGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);   // 关闭光照
		m_pFixedLineGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);  // 打开深度测试
		m_pFixedLineGeometry->getOrCreateStateSet()->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);	// 直线平滑
		m_pFixedLineGeometry->getOrCreateStateSet()->setRenderBinToInherit();
	}

	void CGradientBillBoardDisplay::DrawFixedGeometry()
	{
		osg::ref_ptr<osg::Vec3Array> _vertexFixedLine = new osg::Vec3Array;
		osg::ref_ptr<osg::Vec4Array> _colorFixedLine = new osg::Vec4Array;

		_vertexFixedLine->push_back(osg::Vec3d(
			0.0,
			m_dLineLen, 0));
		_vertexFixedLine->push_back(osg::Vec3d(
			0.0,
			m_dLineLen - m_dQuadHeight, 0));
		_vertexFixedLine->push_back(osg::Vec3d(
			m_dQuadWidth,
			m_dLineLen - m_dQuadHeight, 0));
		_vertexFixedLine->push_back(osg::Vec3d(
			m_dQuadWidth,
			m_dLineLen, 0));

		for (int i = 0; i < 4; ++i)
		{
			_colorFixedLine->push_back(osg::Vec4d(1.0, 0.5, 0.0, 1.0));
		}

		_vertexFixedLine->push_back(osg::Vec3d(
			0.0 ,
			m_dLineLen, 0));
		_vertexFixedLine->push_back(osg::Vec3d(
			0.0,
			m_dLineLen - m_dQuadHeight, 0));		
		_vertexFixedLine->push_back(osg::Vec3d(
			m_dQuadWidth,
			m_dLineLen - m_dQuadHeight, 0));
		_vertexFixedLine->push_back(osg::Vec3d(
			m_dQuadWidth,
			m_dLineLen, 0));

		for (int i = 0; i < 4; ++i)
		{
			_colorFixedLine->push_back(g_v4dBackColor);
		}

		m_pFixedLineGeometry->setColorArray(_colorFixedLine);   // 设置颜色
		m_pFixedLineGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		m_pFixedLineGeometry->setVertexArray(_vertexFixedLine);
		m_pFixedLineGeometry->getOrCreateStateSet()->setAttribute(m_pLineWidth.get(), osg::StateAttribute::ON);//设置线宽
	}

	CGradientBillBoardDisplay::CGradientBillBoardDisplay(osgEarth::MapNode *pMapNode, const osgEarth::GeoPoint &position, 
		float characterSize/* = 13.0*/, int textRow /* = 4*/, 
		double lineLen /*= 55.0*/, double characterNum /*= 9.5*/)
		: osgEarth::Annotation::OrthoNode(pMapNode, position)
	{
		// 确定绘制的尺寸
		m_dLineLen = lineLen;
		m_dCharacterSize = characterSize;
		m_dQuadWidth = characterSize * characterNum + g_dMargin * 2.0;
		m_dQuadHeight = g_dMargin * 2.0 + characterSize * textRow + textRow - 1;
		m_iTextRow = textRow;
		CreateElement();
		DrawFixedGeometry();
		CreateText();
	}

	void CGradientBillBoardDisplay::CreateText()
	{
		m_vecText.clear();
		osg::ref_ptr<osgText::Font> _pTextFont = osgText::readFontFile(FeUtil::CFileReg::Instance()->GetDataPath().append("/fonts/fonts/simkai.ttf"));
		for (int i = 0; i < m_iTextRow; ++i)
		{
			osg::ref_ptr<osgText::Text> pText = new osgText::Text();
			m_pObjectGroup->addChild(pText);
			pText->setFont(_pTextFont);
			pText->setFontResolution(32.0f, 32.0f);
			pText->setColor(g_v4dTextColor);
			pText->setAlignment(osgText::Text::LEFT_CENTER);// 设置为居中
			pText->setBackdropType(osgText::Text::OUTLINE);//设置背景类型
			pText->setBackdropColor(g_v4dBackColor);
			pText->setAxisAlignment(osgText::Text::XY_PLANE);// 设置文字方向为屏幕
			pText->setDrawMode(osgText::Text::TEXT/*| osgText::Text::BOUNDINGBOX*/);//设置绘制模式为填充包围盒
			pText->setBackdropImplementation(osgText::Text::NO_DEPTH_BUFFER);
			// osgText::Text turns on depth writing by default, even if you turned it off.
			pText->setEnableDepthWrites(true);
			pText->setAutoRotateToScreen(false);
			pText->setColorGradientMode(osgText::Text::SOLID);
			pText->setPosition(osg::Vec3d(
				g_dMargin,
				m_dLineLen - m_dCharacterSize*i - m_dCharacterSize/ 2.0 - g_dMargin - i, 0));
			pText->setCharacterSize(m_dCharacterSize);  
			m_vecText.push_back(pText);
		}
	}

	void CGradientBillBoardDisplay::SetText(int textRow, std::string strText, osgText::String::Encoding text_encoding /*= osgText::String::ENCODING_UTF8*/)
	{
		if (m_vecText.size() >= textRow)
		{
			m_vecText[textRow - 1]->setText(strText, text_encoding);
		}
	}

	void CGradientBillBoardDisplay::SetVisible(bool bShow)
	{
		if (bShow)
		{
			this->setNodeMask(0xffffffff);
		}
		else
		{
			this->setNodeMask(0x00000000);
		}
	}

	bool CGradientBillBoardDisplay::IsVisible()
	{
		return this->getNodeMask() == 0xffffffff;
	}

}
