#include <FeMeasure/BillBoardDisplay.h>

#include <osg/Depth>
#include <osg/LineStipple>
#include <osgDB/ConvertUTF>
#include <osgEarthAnnotation/AnnotationUtils>
#include <FeUtils/PathRegistry.h>

namespace FeMeasure
{
	void CMeasuringInfoDisplay::SetOrCreateString( unsigned int unRow, const std::string& str )
	{
		if(unRow >= StrList().size())
		{
			StrList().resize(unRow+1);
		}
		
		if(SetString(unRow, str))
		{
			StringChanged();
		}
	}

	std::string CMeasuringInfoDisplay::GetMultiString()
	{
		std::string multiText;

		STR_INFO_LIST::iterator iter = StrList().begin();
		while(iter != StrList().end())
		{
			multiText += (*(iter++) + "\n");
		}

		return multiText;
	}

	double CMeasuringInfoDisplay::GetMaxStringLen()
	{
		double maxLen = 0;

		STR_INFO_LIST::iterator iter = StrList().begin();
		while(iter != StrList().end())
		{
			if( iter->length() > maxLen )
			{
				maxLen = iter->length();
			}
			iter++;
		}

		return maxLen;
	}

	void CMeasuringInfoDisplay::SetPosition( const osg::Vec3& vecPosLLH )
	{
		if(m_vecPosLLH != vecPosLLH)
		{
			m_vecPosLLH = vecPosLLH; 
			PositionChanged(); 
		}
	}

	bool CMeasuringInfoDisplay::SetString( unsigned int unIndex, const std::string& str )
	{
		if((unIndex < StrList().size()) && (0 != StrList().at(unIndex).compare(str)))
		{
			StrList().at(unIndex) = str; 
			return true;
		}
		return false;
	}

}


namespace FeMeasure
{
	osg::Vec4d g_v4dLineColor1(1.0, 0.5, 0.0, 1.0);
	osg::Vec4d g_v4dBackColor1(0.7, 0.7, 0.7, 1.0);
	osg::Vec4d g_v4dTextColor1(0.0, 0.0, 0.0, 1.0);
	double g_dMargin1 = 2.0;

	CDisplayByBillBoard::CDisplayByBillBoard( osgEarth::MapNode *pMapNode, float characterSize/* = 13.0*/)
	{
		m_rpBillboardNode = new osgEarth::Annotation::OrthoNode(pMapNode, osgEarth::GeoPoint());
		m_dCharacterSize = characterSize;
		m_dLineLen = 55;

		CreateElement();
		CreateText();
	}

	void CDisplayByBillBoard::SetVisible(bool bShow)
	{
		if(m_rpBillboardNode.valid())
		{
			m_rpBillboardNode->setNodeMask(bShow?0xffffffff:0x00000000);
		}
	}

	void CDisplayByBillBoard::CalcQuadSize()
	{
		if(StrList().empty()) return;

		/// 获取最大字符串长度，字符串个数
		double dTextRow = StrList().size() + 1;
		
		double dMaxCharacterLen = GetMaxStringLen()/2;
		m_dQuadWidth = m_dCharacterSize * dMaxCharacterLen + g_dMargin1 * 2.0;
		m_dQuadHeight = m_dCharacterSize * dTextRow + g_dMargin1 * 2.0;
	}

	void CDisplayByBillBoard::CreateElement()
	{
		if(!m_rpBillboardNode.valid()) return;

		m_pObjectGroup = new osg::Group;
		m_rpBillboardNode->getAttachPoint()->addChild(m_pObjectGroup);

		m_pFixedLineGeometry = new osg::Geometry;
		m_pFixedLineGeode = new osg::Geode;

		m_pFixedLineGeode->setComputeBoundingSphereCallback(new osgEarth::Annotation::ControlPointCallback());
		m_pObjectGroup->addChild(m_pFixedLineGeode);
		osg::StateSet* stateSet = m_pFixedLineGeode->getOrCreateStateSet();
		stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), 1);

		m_pLineWidth = new osg::LineWidth(float(1.0));
		m_pFixedLineGeode->addDrawable(m_pFixedLineGeometry);

		m_pFixedLineGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 5));    // 左边竖条
		m_pFixedLineGeometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 5, 4));    // 左边竖条中心的三角形

		m_pFixedLineGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		m_pFixedLineGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);   // 关闭光照
		m_pFixedLineGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);  // 打开深度测试
		m_pFixedLineGeometry->getOrCreateStateSet()->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);	// 直线平滑
		m_pFixedLineGeometry->getOrCreateStateSet()->setRenderBinToInherit();

		m_pFixedStippleLineGeometry = new osg::Geometry;
		m_pFixedStippleLineGeode = new osg::Geode;

		m_pFixedStippleLineGeode->setComputeBoundingSphereCallback(new osgEarth::Annotation::ControlPointCallback());
		m_pObjectGroup->addChild(m_pFixedStippleLineGeode);
		stateSet = m_pFixedStippleLineGeode->getOrCreateStateSet();
		stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), 1);

		m_pFixedStippleLineGeode->addDrawable(m_pFixedStippleLineGeometry);

		m_pFixedStippleLineGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 2));    

		m_pFixedStippleLineGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		m_pFixedStippleLineGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);   // 关闭光照
		m_pFixedStippleLineGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);  // 打开深度测试
		m_pFixedStippleLineGeometry->getOrCreateStateSet()->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);	// 直线平滑
		m_pFixedStippleLineGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::LineStipple(1, 0xf0f0), osg::StateAttribute::ON);
		m_pFixedStippleLineGeometry->getOrCreateStateSet()->setRenderBinToInherit();
	}

	void CDisplayByBillBoard::CreateText()
	{
		if(!m_rpText.valid())
		{
			osg::ref_ptr<osgText::Font> _pTextFont = osgText::readFontFile(
				FeUtil::CFileReg::Instance()->GetDataPath().append("/fonts/fonts/simkai.ttf"));

			m_rpText = new osgText::Text();
			m_pObjectGroup->addChild(m_rpText);
			m_rpText->setFont(_pTextFont);
			m_rpText->setFontResolution(32.0f, 32.0f);
			m_rpText->setColor(g_v4dTextColor1);
			m_rpText->setAlignment(osgText::Text::LEFT_TOP);// 设置为居中
			m_rpText->setBackdropType(osgText::Text::OUTLINE);//设置背景类型
			m_rpText->setBackdropColor(g_v4dBackColor1);
			m_rpText->setAxisAlignment(osgText::Text::XY_PLANE);// 设置文字方向为屏幕
			m_rpText->setDrawMode(osgText::Text::TEXT/*| osgText::Text::BOUNDINGBOX*/);//设置绘制模式为填充包围盒
			m_rpText->setBackdropImplementation(osgText::Text::NO_DEPTH_BUFFER);
			// osgText::Text turns on depth writing by default, even if you turned it off.
			m_rpText->setEnableDepthWrites(true);
			m_rpText->setAutoRotateToScreen(false);
			m_rpText->setColorGradientMode(osgText::Text::SOLID);
			m_rpText->setCharacterSize(m_dCharacterSize); 

			m_rpText->setPosition(osg::Vec3d(-m_dQuadWidth / 2.0 + g_dMargin1,
				m_dLineLen + m_dQuadHeight - m_dCharacterSize/ 2.0 - g_dMargin1, 0));
		}
	}

	void CDisplayByBillBoard::DrawFixedGeometry()
	{
		osg::ref_ptr<osg::Vec3dArray> _vertexFixedStippleLine = new osg::Vec3dArray;
		osg::ref_ptr<osg::Vec4dArray> _colorFixedStippleLine = new osg::Vec4dArray;

		_vertexFixedStippleLine->push_back(osg::Vec3d(0.0, 0.0, 0));
		_vertexFixedStippleLine->push_back(osg::Vec3d(0.0, m_dLineLen, 0));

		_colorFixedStippleLine->push_back(g_v4dLineColor1);
		_colorFixedStippleLine->push_back(g_v4dLineColor1);

		m_pFixedStippleLineGeometry->setColorArray(_colorFixedStippleLine);   // 设置颜色
		m_pFixedStippleLineGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		m_pFixedStippleLineGeometry->setVertexArray(_vertexFixedStippleLine);
		m_pFixedStippleLineGeometry->getOrCreateStateSet()->setAttribute(m_pLineWidth.get(), osg::StateAttribute::ON);//设置线宽

		osg::ref_ptr<osg::Vec3dArray> _vertexFixedLine = new osg::Vec3dArray;
		osg::ref_ptr<osg::Vec4dArray> _colorFixedLine = new osg::Vec4dArray;

		_vertexFixedLine->push_back(osg::Vec3d(-m_dQuadWidth / 2.0, m_dLineLen, 0));
		_vertexFixedLine->push_back(osg::Vec3d(-m_dQuadWidth / 2.0, m_dLineLen + m_dQuadHeight, 0));
		_vertexFixedLine->push_back(osg::Vec3d(m_dQuadWidth / 2.0, m_dLineLen + m_dQuadHeight, 0));
		_vertexFixedLine->push_back(osg::Vec3d(m_dQuadWidth / 2.0, m_dLineLen, 0));
		_vertexFixedLine->push_back(osg::Vec3d(- m_dQuadWidth / 2.0, m_dLineLen, 0));

		for (int i = 0; i < 5; ++i)
		{
			_colorFixedLine->push_back(osg::Vec4d(1.0, 0.5, 0.0, 1.0));
		}

		_vertexFixedLine->push_back(osg::Vec3d(
			- m_dQuadWidth / 2.0,
			m_dLineLen, 0));
		_vertexFixedLine->push_back(osg::Vec3d(
			- m_dQuadWidth / 2.0,
			m_dLineLen + m_dQuadHeight, 0));		
		_vertexFixedLine->push_back(osg::Vec3d(
			m_dQuadWidth / 2.0,
			m_dLineLen + m_dQuadHeight, 0));
		_vertexFixedLine->push_back(osg::Vec3d(
			m_dQuadWidth / 2.0,
			m_dLineLen, 0));

		for (int i = 0; i < 4; ++i)
		{
			_colorFixedLine->push_back(g_v4dBackColor1);
		}

		m_pFixedLineGeometry->setColorArray(_colorFixedLine);   // 设置颜色
		m_pFixedLineGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		m_pFixedLineGeometry->setVertexArray(_vertexFixedLine);
		m_pFixedLineGeometry->getOrCreateStateSet()->setAttribute(m_pLineWidth.get(), osg::StateAttribute::ON);//设置线宽
	}

	void CDisplayByBillBoard::PositionChanged()
	{
		if(m_rpBillboardNode.valid() && m_rpBillboardNode->getMapNode())
		{
			m_rpBillboardNode->setPosition(osgEarth::GeoPoint(
				m_rpBillboardNode->getMapNode()->getMapSRS(), PosLLH(),osgEarth::ALTMODE_ABSOLUTE));
		}
	}

	void CDisplayByBillBoard::StringChanged()
	{
		if(m_rpText.valid())
		{
			CalcQuadSize();
			DrawFixedGeometry();

			m_rpText->setPosition(osg::Vec3d(-m_dQuadWidth / 2.0 + m_dCharacterSize/ 2.0 + g_dMargin1,
				m_dLineLen + m_dQuadHeight - m_dCharacterSize/ 2.0 - g_dMargin1, 0));
			m_rpText->setText(GetMultiString(), osgText::String::ENCODING_UTF8);
		}
	}

}