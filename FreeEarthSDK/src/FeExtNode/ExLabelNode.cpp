#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>
#include <FeUtils/CoordConverter.h>
#include <FeExtNode/ExLabelNode.h>


namespace FeExtNode
{
	CExLabelItemOption& CExLabelItemOption::operator=( const CExLabelItemOption& opt )
	{
		this->m_fWidth = opt.m_fWidth;
		this->m_fHeight = opt.m_fHeight;
		this->m_strText = opt.m_strText;
		this->m_strFontName = opt.m_strFontName;
		this->m_fFontSize = opt.m_fFontSize;
		this->m_vecTextColor = opt.m_vecTextColor;
		this->m_vecBackgroundColor = opt.m_vecBackgroundColor;
		this->m_rpBackgroundImage = opt.m_rpBackgroundImage;
		this->m_rpHoverImage = opt.m_rpHoverImage;
		this->m_textAlign = opt.m_textAlign;
		this->m_strImagePath = opt.m_strImagePath;
		return *this;
	}

	CExLabelItemOption::CExLabelItemOption()
	{
		m_fFontSize = 12;
		m_fWidth = 100;
		m_fHeight = 30;
		m_vecBackgroundColor = osg::Vec4(0.3, 0, 1, 0.5);
		m_strText = "Label";
		m_vecTextColor = osg::Vec4(1, 1, 1, 1);
		m_textAlign = ALIGN_CENTER;
		m_strImagePath = "";
	}

	CExLabelItemOption::~CExLabelItemOption()
	{

	}
}

namespace FeExtNode
{
	
	CExLabelItemNode::CExLabelItemNode( FeUtil::CRenderContext* pRender, CExLabelItemOption* opt ) 
		:CExternNode(opt)
		  , m_opRenderContext(pRender)
		  , m_vecLT(0.5, 0.5)
		  , m_bShowChild(true)
		  , m_curViewPort(1,1)
	{
		m_rpGeodeRoot = new osg::Geode;
		addChild(m_rpGeodeRoot);

		CreateTextNode();

		CreateQuadBackgroundNode();
		
		getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED| osg::StateAttribute::OVERRIDE);
		getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN); 
		setCullingActive(false);
	}

	void CExLabelItemNode::SetPosition( const osg::Vec2& vecPosNorm )
	{
		//if(m_vecLT != vecPosNorm) 
		{
			m_vecLT = vecPosNorm;

			UpdatePosition();
		}
	}

	void CExLabelItemNode::SetChildItemVisible( bool bShow )
	{
		if(m_bShowChild != bShow)
		{
			m_bShowChild = bShow;

            CExLabelItemNodeList::iterator it = m_listChildNode.begin();
            while(it != m_listChildNode.end())
            {
                CExLabelItemNode* pNode = *it;
                if(pNode)
                {
                    pNode->SetVisible(bShow);
                }
                it++;
            }
		}
	}

	bool CExLabelItemNode::AddChildItem( CExLabelItemNode* pItem )
	{
		if(pItem)
		{
			/// 计算子项位置
			osg::Vec2d vecLT;
			
			if(m_listChildNode.empty())
			{
				vecLT = m_vecLT + osg::Vec2(GetWidth() / m_curViewPort.x(), 0);
			}
			else
			{
				vecLT = m_listChildNode.back()->GetPosition() 
					+ osg::Vec2(0, - m_listChildNode.back()->GetHeight() / m_curViewPort.y());
			}
			
			/// 添加
			this->addChild(pItem);
			m_listChildNode.push_back(pItem);
			pItem->SetPosition(vecLT);

			return true;
		}

		return false;
	}

	FeUtil::FEID CExLabelItemNode::AddChildItem( const std::string& strItemText )
	{
		osg::ref_ptr<CExLabelItemOption> opt = new CExLabelItemOption;
		
		if(m_listChildNode.empty())
		{
			*opt = *GetOption();
		}
		else
		{
			*opt = *(m_listChildNode.back()->GetOption());
		}
		
		opt->Text() = strItemText;

		osg::ref_ptr<CExLabelItemNode> node = new CExLabelItemNode(m_opRenderContext.get(), opt);
		AddChildItem(node);

		if(!m_bShowChild)
			node->SetVisible(false);

		return node->GetID();
	}

	bool CExLabelItemNode::isPtIn( const osg::Vec2& vecPosScreen )
	{
		osg::Vec2 vecPosNorm(vecPosScreen.x()/m_curViewPort.x(), vecPosScreen.y()/m_curViewPort.y());

		/// 和四边形顶点作比较
		if(m_rpQuadVertexes->size() >= 4)
		{
			return ( (vecPosNorm.x() > m_rpQuadVertexes->at(0).x() && vecPosNorm.x() < m_rpQuadVertexes->at(1).x())
				&& ( vecPosNorm.y() > m_rpQuadVertexes->at(3).y() && vecPosNorm.y() < m_rpQuadVertexes->at(0).y())
				);
		}
		
		return false;
	}
		
	CExLabelItemNode* CExLabelItemNode::GetItemByPos( const osg::Vec2& vecPosScreen )
	{
		if(!GetVisible())
		{
			return 0;
		}

		if(isPtIn(vecPosScreen))
		{
			return this;
		}

		/// 为了提高效率，可考虑采用根据坐标区域决定是否继续查找孩子节点
		/// 或者确定查找顺序。 目前为顺序查找

        CExLabelItemNodeList::iterator it = m_listChildNode.begin();
        while(it != m_listChildNode.end())
        {
            CExLabelItemNode* pNode = *it;
            if(pNode)
            {
                CExLabelItemNode* pItem = pNode->GetItemByPos(vecPosScreen);
                if(pItem)
                {
                    return pItem;
                }
            }
            it++;
        }

        //for each(CExLabelItemNode* var in m_listChildNode)
        //{
        //	CExLabelItemNode* pItem = var->GetItemByPos(vecPosScreen);
        //	if(pItem)
        //	{
        //		return pItem;
        //	}
        //}

		return 0;
	}

	void CExLabelItemNode::GetDiaplayMaxSize( float& fMaxWidth, float& fMaxHeight )
	{
		if(!GetVisible())
		{
			fMaxWidth = fMaxHeight = 0;
			return;
		}

		// 目前只计算一级目录
		//if(m_listChildNode.empty() || !m_bShowChild)
		{
			fMaxWidth = GetWidth() / m_curViewPort.x();
			fMaxHeight = GetHeight() / m_curViewPort.y();
			return;
		}

		float fChildW, fChildH;
		fMaxHeight = fMaxWidth = 0;

        CExLabelItemNodeList::iterator it = m_listChildNode.begin();
        while(it != m_listChildNode.end())
        {
            CExLabelItemNode* pNode = *it;
            if(pNode)
            {
                pNode->GetDiaplayMaxSize(fChildW, fChildH);

                fMaxHeight += fChildH;  // 高度累加

                if(fMaxWidth < fChildW) // 宽度取最大值
                {
                    fMaxWidth = fChildW;
                }
            }
            it++;
        }

        //for each (CExLabelItemNode* var in m_listChildNode)
        //{
        //	var->GetDiaplayMaxSize(fChildW, fChildH);

        //	fMaxHeight += fChildH;  // 高度累加

        //	if(fMaxWidth < fChildW) // 宽度取最大值
        //	{
        //		fMaxWidth = fChildW;
        //	}
        //}

		/// 再加上该父节点的宽度
		fMaxWidth += GetWidth() / m_curViewPort.x();
	}

	CExLabelItemNode* CExLabelItemNode::GetChildItem( FeUtil::FEID nodeID )
	{
        CExLabelItemNodeList::iterator it = m_listChildNode.begin();
        while(it != m_listChildNode.end())
        {
            CExLabelItemNode* pNode = *it;
            if(pNode)
            {
                if(nodeID == pNode->GetID())
                {
                    return pNode;
                }
            }
            it++;
        }

        //for each(CExLabelItemNode* var in m_listChildNode)
        //{
        //	if(nodeID == var->GetID())
        //	{
        //		return var;
        //	}
        //}

		return 0;
	}

	bool CExLabelItemNode::RemoveChildItem( FeUtil::FEID nodeID )
	{
		CExLabelItemNode* pItem = GetChildItem(nodeID);

		if(pItem && this->removeChild(pItem))
		{
			m_listChildNode.remove(pItem);

			UpdatePosition();

			return true;
		}

		return false;
	}

	void CExLabelItemNode::CreateTextNode()
	{
		m_text = new osgText::Text;
		m_rpGeodeRoot->addDrawable(m_text);

		m_font = osgText::readFontFile(FeFileReg->GetFullPath(GetOption()->FontName()));

		if(!m_font.valid())
		{
			//m_font = osgText::readFontFile(FeFileReg->GetFullPath("fonts/msyh.ttf"));
			m_font = osgText::readFontFile(FeFileReg->GetFullPath("fonts/simhei.ttf"));
		}

		m_text->setFont(m_font);
		m_text->setText(GetOption()->Text(), osgText::String::ENCODING_UTF8);
		m_text->setColor(GetOption()->TextColor());
		m_text->setAlignment(osgText::TextBase::LEFT_CENTER);
		m_text->setFontResolution(64.0f,64.0f);
		m_text->setDataVariance(osg::Object::DYNAMIC);
		m_text->setBackdropType(osgText::Text::NONE);
		m_text->setColorGradientMode(osgText::Text::SOLID);
	}

	void CExLabelItemNode::CreateQuadBackgroundNode()
	{
		m_rpGeomQuad = new osg::Geometry;
		m_rpGeodeRoot->addDrawable(m_rpGeomQuad);
		
		m_rpQuadVertexes = new osg::Vec3Array(4);
		m_rpGeomQuad->setVertexArray(m_rpQuadVertexes);
		m_rpGeomQuad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, m_rpQuadVertexes->size()));

		osg::ref_ptr<osg::Vec3Array> vecNorm = new osg::Vec3Array;
		vecNorm->push_back(osg::Vec3(0, 0, 1));
		vecNorm->at(0).normalize();
		m_rpGeomQuad->setNormalArray(vecNorm, osg::Array::BIND_OVERALL);

		m_rpVecBkColor = new osg::Vec4Array;
		m_rpVecBkColor->push_back(GetOption()->BackgroundColor());
		m_rpGeomQuad->setColorArray(m_rpVecBkColor, osg::Array::BIND_OVERALL);
		m_rpGeomQuad->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

		m_rpBkImageTexture = new osg::Texture2D;
		m_rpGeomQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0 ,m_rpBkImageTexture, osg::StateAttribute::ON);
		m_rpBkImageTexture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
		m_rpBkImageTexture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);

		osg::ref_ptr<osg::Vec2Array> TexCoords = new osg::Vec2Array;
		TexCoords->push_back(osg::Vec2(0, 1));
		TexCoords->push_back(osg::Vec2(1, 1));
		TexCoords->push_back(osg::Vec2(1, 0));
		TexCoords->push_back(osg::Vec2(0, 0));

		m_rpGeomQuad->setTexCoordArray(0, TexCoords);
		
		SetBackgroundImage(GetOption()->BackgroundImagePath());

// 		if(GetOption()->BackgroundImage().valid())
// 		{
// 			m_rpBkImageTexture->setImage(GetOption()->BackgroundImage());
// 		}
	}

	void CExLabelItemNode::UpdatePosition()
	{
		UpdateTextAndViewPort();

		UpdateBackgroundQuad();
		
		if(!m_listChildNode.empty())
		{
			/// 更新子项位置

			osg::Vec2 vecChildPos = m_vecLT + osg::Vec2(GetOption()->Width() / m_curViewPort.x(), 0);

            CExLabelItemNodeList::iterator it = m_listChildNode.begin();
            while(it != m_listChildNode.end())
            {
                CExLabelItemNode* pNode = *it;
                if(pNode)
                {
                    pNode->SetPosition(vecChildPos);
                    vecChildPos.y() -= (pNode->GetOption()->Height() / m_curViewPort.y());
                }
                it++;
            }

            //for each(CExLabelItemNode* var in m_listChildNode)
            //{
            //	(*var).SetPosition(vecChildPos);
            //	vecChildPos.y() -= ((*var).GetOption()->Height() / m_curViewPort.y());
            //}
		}
	}

	void CExLabelItemNode::UpdateTextAndViewPort()
	{
		float screenWidth, screenHeight;
		GetCurrentViewPort(screenWidth, screenHeight);

		/// 根据对齐方式，调整文本位置
		osg::Vec2 vecTextPos;
		const float widthNorm = GetOption()->Width() / screenWidth;
		const float heightNorm = GetOption()->Height() / screenHeight;

		switch(GetOption()->Alignment())
		{
		case CExLabelItemOption::ALIGN_LEFT:
			{
				m_text->setAlignment(osgText::TextBase::LEFT_CENTER);
				vecTextPos = m_vecLT + osg::Vec2(0, -heightNorm/2);
			}
			break;
		case CExLabelItemOption::ALIGN_RIGHT:
			{
				m_text->setAlignment(osgText::TextBase::RIGHT_CENTER);
				vecTextPos = m_vecLT + osg::Vec2(widthNorm, -heightNorm/2);
			}
			break;
		case CExLabelItemOption::ALIGN_CENTER:
			{
				m_text->setAlignment(osgText::TextBase::CENTER_CENTER);
				vecTextPos = m_vecLT + osg::Vec2(widthNorm/2, -heightNorm/2);
			}
			break;
		}

		m_text->setPosition(osg::Vec3(vecTextPos, 0.0));


		/// 设置字体大小，若视口发生变化，需调整字体比例，避免拉伸
		float fWRate = screenWidth / m_curViewPort.x();
		float fHRate = screenHeight / m_curViewPort.y();

		if(std::abs(fWRate-1) > 0.000001 || std::abs(fHRate-1) > 0.000001)
		{
			m_text->setCharacterSize(GetOption()->FontSize() / screenHeight, 
				m_text->getCharacterAspectRatio()*fWRate/fHRate);

			m_text->update();

			/// 记录当前视口
			m_curViewPort.set(screenWidth, screenHeight);
		}
		else
		{
			m_text->setCharacterSize(GetOption()->FontSize() / screenHeight);
		}
	}

	void CExLabelItemNode::UpdateBackgroundQuad()
	{
		const float fDepth = - 0.01; // 四边形在文本之下
		osg::Vec3 vecLT = osg::Vec3(m_vecLT, fDepth);
		const float widthNorm = GetOption()->Width() / m_curViewPort.x();
		const float heightNorm = GetOption()->Height() / m_curViewPort.y();

		if(m_rpQuadVertexes->size() >= 4)
		{
			m_rpQuadVertexes->at(0) = (vecLT);
			m_rpQuadVertexes->at(1) = (vecLT + osg::Vec3(widthNorm, 0, 0));
			m_rpQuadVertexes->at(2) = (vecLT + osg::Vec3(widthNorm, -heightNorm, 0));
			m_rpQuadVertexes->at(3) = (vecLT + osg::Vec3(0, -heightNorm, 0));

			m_rpGeomQuad->dirtyDisplayList();
			m_rpGeomQuad->dirtyBound();
		}
	}

	void CExLabelItemNode::GetCurrentViewPort( float& fWidth, float& fHeight )
	{
		fWidth = 640;
		fHeight = 480;

		if (m_opRenderContext->GetCamera() && m_opRenderContext->GetCamera()->getViewport())
		{
			osg::Viewport* viewport = m_opRenderContext->GetCamera()->getViewport();
			fWidth = viewport->width();
			fHeight = viewport->height();
		}

		if (std::abs(fWidth) < 0.000001 || std::abs(fHeight)<0.000001)
		{
			osg::notify(osg::WARN)<<"场景视镜器不视口宽或高不能为0";
			fWidth = 640;
			fHeight = 480;
			return;
		}
	}

	const float CExLabelItemNode::GetCurrentViewPortWidth()
	{
		float fWidth, fHeight;
		GetCurrentViewPort(fWidth, fHeight);
		return fWidth;
	}

	const float CExLabelItemNode::GetCurrentViewPortHeight()
	{
		float fWidth, fHeight;
		GetCurrentViewPort(fWidth, fHeight);
		return fHeight;
	}
	void CExLabelItemNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}
	
	CExLabelItemOption* CExLabelItemNode::GetOption()
	{
		return dynamic_cast<CExLabelItemOption*>(m_rpOptions.get());
	}


	const std::string& CExLabelItemNode::GetFontName()
	{
		return GetOption()->FontName();
	}


	void CExLabelItemNode::SetFontName( const std::string& fontName )
	{
	}

	std::string CExLabelItemNode::GetText()
	{
		return GetOption()->Text();
	}

	void CExLabelItemNode::SetText( const std::string& strText )
	{
		if(GetOption()->Text() != strText)
		{
			GetOption()->Text() = strText;

			m_text->setText(strText, osgText::String::ENCODING_UTF8);
		}
	}

	const osg::Vec4& CExLabelItemNode::GetTextColor()
	{
		return GetOption()->TextColor();
	}

	void CExLabelItemNode::SetTextColor( const osg::Vec4& vecColor )
	{
		if(GetOption()->TextColor() != vecColor)
		{
			GetOption()->TextColor() = vecColor;

			m_text->setColor(vecColor);
		}
	}

	const osg::Vec4& CExLabelItemNode::GetBackgroundColor()
	{
		return GetOption()->BackgroundColor();
	}

	void CExLabelItemNode::SetBackgroundColor( const osg::Vec4& vecColor )
	{
		if(GetOption()->BackgroundColor() != vecColor)
		{
			GetOption()->BackgroundColor() = vecColor;

			if(m_rpVecBkColor->size() >= 1)
			{
				m_rpVecBkColor->at(0) = vecColor;
				m_rpGeomQuad->dirtyDisplayList();
				m_rpGeomQuad->dirtyBound();
			}
		}
	}

	void CExLabelItemNode::SetBackgroundImage( const std::string& strPath )
	{
		osg::ref_ptr<osg::Image> rpImage = osgDB::readImageFile(strPath);
		GetOption()->BackgroundImagePath() = strPath;
		SetBackgroundImage(rpImage);
	}

	void CExLabelItemNode::SetBackgroundImage( osg::Image* pImage )
	{
		if(pImage)
		{
			GetOption()->BackgroundImage() = pImage;

			m_rpBkImageTexture->setImage(pImage);
		}
	}

	std::string CExLabelItemNode::GetBackgroundImage()
	{
		return GetOption()->BackgroundImagePath();
	}

	void CExLabelItemNode::SetHoverImage( osg::Image* pImage )
	{
		if(pImage)
		{
			GetOption()->HoverImage() = pImage;

			m_rpBkImageTexture->setImage(pImage);
		}
	}

	osg::Image* CExLabelItemNode::GetHoverImage()
	{
		return GetOption()->HoverImage();
	}

	float CExLabelItemNode::GetFontSize()
	{
		return GetOption()->FontSize();
	}

	void CExLabelItemNode::SetFontSize( const float& fSize )
	{
		if(GetOption()->FontSize() != fSize)
		{
			GetOption()->FontSize() = fSize;

			m_text->setCharacterSize(fSize / GetCurrentViewPortHeight());
		}
	}

	float CExLabelItemNode::GetWidth()
	{
		return GetOption()->Width();
	}

	void CExLabelItemNode::SetWidth( const float& fWidth )
	{
		if(GetOption()->Width() != fWidth)
		{
			GetOption()->Width() = fWidth;

			UpdatePosition();
		}
	}

	float CExLabelItemNode::GetHeight()
	{
		return GetOption()->Height();
	}

	void CExLabelItemNode::SetHeight( const float& fHeight )
	{
		if(GetOption()->Height() != fHeight)
		{
			GetOption()->Height() = fHeight;

			UpdatePosition();
		}
	}

	CExLabelItemOption::E_TEXT_ALIGNMENT CExLabelItemNode::GetTextAlign()
	{
		return GetOption()->Alignment();
	}

	void CExLabelItemNode::SetTextAlign( CExLabelItemOption::E_TEXT_ALIGNMENT align )
	{
		if(align != GetOption()->Alignment())
		{
			GetOption()->Alignment() = align;

			UpdateTextAndViewPort();
		}
	}

	FeExtNode::CExLabelItemNodeList CExLabelItemNode::GetChildList()
	{
		return m_listChildNode;
	}

	void CExLabelItemNode::RemoveBackgroundImage()
	{
		GetOption()->BackgroundImagePath() = "";
		GetOption()->BackgroundImage() = NULL;
		m_rpGeomQuad->getOrCreateStateSet()->removeTextureAttribute(0, osg::StateAttribute::TEXTURE);
		m_rpBkImageTexture = new osg::Texture2D;
		m_rpGeomQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0 ,m_rpBkImageTexture, osg::StateAttribute::ON);
	}

}


namespace FeExtNode
{
	CExLabelNode::CExLabelNode( FeUtil::CRenderContext* pRender, CExLabelNodeOption* opt ) 
		: CExternNode(opt)
		, m_opRenderContext(pRender)
		, m_bEnableBinding(false)
		, m_bShowItem(true)
		, m_bLineChanged(false)
		, m_eLinkType(E_LINK_LT)
		, m_bClampToTerrain(false)
	{
		/// 创建相机结点
		CreateCameraNode();

		/// 创建标头
		CreateTitle();

		/// 创建连接线节点
		CreateLinkLine();
		
		/// 初始位置
		InitPosition();
	}

	
	CExLabelNode::~CExLabelNode()
	{
		m_listItemNode.clear();
	}

	bool CExLabelNode::AddLabelItem( CExLabelItemNode* pItem )
	{
		if(pItem)
		{
			if(m_listItemNode.empty())
			{
				return false;
			}

			/// 计算子项位置
			osg::Vec2d vecLT = m_listItemNode.back()->GetPosition();
			vecLT.y() -= (m_listItemNode.back()->GetOption()->Height() / pItem->GetCurrentViewPortHeight());

			/// 添加
			m_rpCameraNode->addChild(pItem);
			m_listItemNode.push_back(pItem);
			pItem->SetPosition(vecLT);
			
			UpdateBindingLine();

			return true;
		}

		return false;
	}

	FeUtil::FEID CExLabelNode::AddLabelItem( const std::string& strItemText )
	{
		osg::ref_ptr<CExLabelItemOption> opt = new CExLabelItemOption();
		
		if(m_listItemNode.empty())
		{
			/// 使用标头相同的属性
			*opt = *(m_rpTitleNode->GetOption());
		}
		else
		{
			/// 使用末尾子项相同的属性
			*opt = *(m_listItemNode.back()->GetOption());
		}

		opt->Text() = strItemText;

		osg::ref_ptr<CExLabelItemNode> node = new CExLabelItemNode(m_opRenderContext.get(), opt);
		AddLabelItem(node);

		//UpdateBindingLine();
		if(!m_bShowItem)
			node->SetVisible(false);
		
		return node->GetID();
	}

	void CExLabelNode::MakePosValid( osg::Vec2d& vecPosNorm )
	{
		/// 获取当前标牌显示的最大宽度与最大高度，进行屏幕边缘处理
		float fMaxWidth = 0, fMaxHeight = 0, fTmpW, fTmpH;
		
		if(!m_bShowItem)
		{
			m_rpTitleNode->GetDiaplayMaxSize(fMaxWidth, fMaxHeight);
		}
		else
		{
            CExLabelItemNodeList::iterator it = m_listItemNode.begin();
            while(it != m_listItemNode.end())
            {
                CExLabelItemNode* pNode = *it;
                if(pNode)
                {
                    pNode->GetDiaplayMaxSize(fTmpW, fTmpH);

                    if(fMaxWidth < fTmpW)
                    {
                        fMaxWidth = fTmpW;
                    }

                    /// 得到整体高度
                    fTmpH += (m_rpTitleNode->GetPosition().y() - pNode->GetPosition().y());

                    if(fMaxHeight < fTmpH)
                    {
                        fMaxHeight = fTmpH;
                    }
                }
				++it;
            }
		}
	
		/// 检验
		if(vecPosNorm.x() < 0)
		{
			vecPosNorm.x() = 0;
		}
		else if(vecPosNorm.x() + fMaxWidth > 1)
		{
			vecPosNorm.x() = 1 - fMaxWidth;
		}
		if(vecPosNorm.y() > 1)
		{
			vecPosNorm.y() = 1;
		}
		else if(vecPosNorm.y() - fMaxHeight <= 0)
		{
			vecPosNorm.y() = fMaxHeight;
		}
	}

	CExLabelItemNode* CExLabelNode::GetLabelItem( FeUtil::FEID nodeID )
	{
        CExLabelItemNodeList::iterator it = m_listItemNode.begin();
        while(it != m_listItemNode.end())
        {
            CExLabelItemNode* pNode = *it;
            if(pNode)
            {
                if(nodeID == pNode->GetID())
                {
                    return pNode;
                }
            }
			++it;
        }

		return 0;
	}

	bool CExLabelNode::RemoveLabelItem( FeUtil::FEID nodeID )
	{
		CExLabelItemNode* pItem = GetLabelItem(nodeID);

		/// 标头节点不能移除
		if(pItem == m_rpTitleNode.get())
		{
			return false;
		}

		if(pItem && m_rpCameraNode->removeChild(pItem))
		{
			m_listItemNode.remove(pItem);
			
			m_bNeedRedraw = true;

			return true;
		}

		return false;
	}

	void CExLabelNode::SetBindingEnable( const bool bEnableBinding )
	{
		if(m_bEnableBinding != bEnableBinding)
		{
			m_bEnableBinding = bEnableBinding;
			m_bNeedRedraw = true;
		}
	}

	bool CExLabelNode::IsBindingEnable()
	{
		return m_bEnableBinding;
	}

	bool CExLabelNode::SetBindingPointLLH( const osg::Vec3d& pt )
	{
		if(GetOption()->BindingLLH() != pt)
		{
			GetOption()->BindingLLH() = pt;

			m_bNeedRedraw = true;
		}

		return true;
	}

	const osg::Vec3d& CExLabelNode::GetBindingPointLLH()
	{
		return GetOption()->BindingLLH();
	}

	bool CExLabelNode::BindingNode( osg::Node* pNode )
	{
		if(pNode)
		{
			m_rpLabelBindingCallback = 0;
			m_rpLabelBindingCallback = new CExLabelBindingNodeCallback(m_opRenderContext.get());
			return m_rpLabelBindingCallback->AddLabelBindNode(this, pNode);
		}
		return false;
	}

	bool CExLabelNode::RemoveBindingNode( osg::Node* pNode )
	{
		if(pNode && m_rpLabelBindingCallback.valid())
		{
			return m_rpLabelBindingCallback->RemoveBindNode(pNode);
		}
		return false;
	}

	bool CExLabelNode::ClickLabel( const osg::Vec2& vecPosScreen, bool bDoubleClick )
	{
		if(bDoubleClick)
		{
			if(m_rpTitleNode->isPtIn(vecPosScreen))
			{
				ShowLabelItem(!m_bShowItem);
				UpdateBindingLine();
				return true;
			}
		}
		else
		{
			/// 为了提高效率，可采用根据位置区域决定查找顺序，目前为顺序查找
			
			for(CExLabelItemNodeList::const_iterator iter = ++(m_listItemNode.begin());
				iter != m_listItemNode.end(); iter++)
			{
				CExLabelItemNode* itemNode = (*iter).get()->GetItemByPos(vecPosScreen);
				if(itemNode)
				{
					/// 显示之前，将兄弟节点的孩子树隐藏
					if(!itemNode->IsChildItemVisible())
					{
						HideBrotherTree(itemNode);
					}

					itemNode->SetChildItemVisible(!itemNode->IsChildItemVisible());
					return true;
				}
			}
		}

		return false;
	}

	CExLabelItemNode* GetParentNode(CExLabelItemNode* pItemNode, CExLabelItemNode* pParentGuess)
	{
		if(!pParentGuess) return 0;

		CExLabelItemNode* pParentReturn = 0;

		CExLabelItemNodeList children = pParentGuess->GetChildList();
		for(CExLabelItemNodeList::iterator iter = children.begin(); iter != children.end(); iter++)
		{
			if(iter->get() == pItemNode)
			{
				pParentReturn = pParentGuess;
			}
			else
			{
				pParentReturn = GetParentNode(pItemNode, iter->get());
			}
			
			if(pParentReturn)
			{
				return pParentReturn;
			}
		}

		return 0;
	}

	CExLabelItemNode* CExLabelNode::FindParentNode(CExLabelItemNode* pItemNode)
	{
		CExLabelItemNode* pParentReturn = 0;

		for(CExLabelItemNodeList::iterator iter = m_listItemNode.begin(); iter != m_listItemNode.end(); iter++)
		{
			if(iter->get() == pItemNode)
			{
				return NULL;
			}
			else
			{
				pParentReturn = GetParentNode(pItemNode, iter->get());
			}
			
			if(pParentReturn)
			{
				return pParentReturn;
			}
		}
		
		return 0;
	}

	void CExLabelNode::HideBrotherTree(CExLabelItemNode* pItemNode)
	{
		/// Find parent node
		CExLabelItemNode* pParentNode = FindParentNode(pItemNode);

		CExLabelItemNodeList children;
		if(pParentNode)
		{
			children = pParentNode->GetChildList();
		}
		else
		{
			children = m_listItemNode;
		}

		for(CExLabelItemNodeList::iterator iter = children.begin(); iter != children.end(); iter++)
		{
			if(iter->get() != pItemNode && iter->get()->IsChildItemVisible())
			{
				iter->get()->SetChildItemVisible(false);
			}
		}
	}

	bool CExLabelNode::IsPtInTitle( const osg::Vec2& vecPosScreen )
	{
		return m_rpTitleNode->isPtIn(vecPosScreen);
	}

	void CExLabelNode::DragLabel( const osg::Vec2& vecCurPosScreen, const osg::Vec2& vecLastPosScreen )
	{
		if(vecCurPosScreen == vecLastPosScreen)
		{
			/// 位置没有发生变化
			return;
		}

		float fViewWidth, fViewHeight;
		m_rpTitleNode->GetCurrentViewPort(fViewWidth, fViewHeight);

		/// 计算相对位移
		osg::Vec2d vecDelta = vecCurPosScreen - vecLastPosScreen;
		vecDelta.x() /= fViewWidth;
		vecDelta.y() /= fViewHeight;

		osg::Vec2d vecLT = vecDelta + m_rpTitleNode->GetPosition();

		/// 检验位置值
		MakePosValid(vecLT);

		/// 设置位置
		SetLabelPosition(vecLT);

		/// 根据绑定点计算合适的连接方式
		UpdateLinkTypeByBindingPoint();

		/// 更新连接线
		UpdateBindingLine();

		m_bLineChanged = true;
	}

	void CExLabelNode::ShowLabelItem( bool bShow )
	{
		if(bShow == m_bShowItem)
		{
			return;
		}

		m_bShowItem = bShow;

		for(CExLabelItemNodeList::iterator iter = ++(m_listItemNode.begin());
			iter != m_listItemNode.end(); iter++)
		{
			(*iter).get()->SetVisible(m_bShowItem);
		}
	}

	CExLabelNodeOption* CExLabelNode::GetOption()
	{
		return dynamic_cast<CExLabelNodeOption*>(m_rpOptions.get());
	}

	void CExLabelNode::CreateCameraNode()
	{
		if(m_rpCameraNode.valid())
		{
			return;
		}

		m_rpCameraNode = new osg::Camera;
		addChild(m_rpCameraNode);

		/// 将整个屏幕投射到【0,1】范围
		m_rpCameraNode->setProjectionMatrix(osg::Matrix::ortho(0, 1, 0, 1, -0.5, 0.5));
		m_rpCameraNode->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		m_rpCameraNode->setViewMatrix(osg::Matrix::identity());
		m_rpCameraNode->setClearMask(GL_DEPTH_BUFFER_BIT);
		m_rpCameraNode->setAllowEventFocus(false);
		m_rpCameraNode->setRenderOrder(osg::Camera::POST_RENDER);
	}

	void CExLabelNode::CreateLinkLine()
	{
		if(m_rpGeomLinkLine.valid())
		{
			return;
		}

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		m_rpCameraNode->addChild(geode);

		m_rpGeomLinkLine = new osg::Geometry;
		geode->addDrawable(m_rpGeomLinkLine);
		
		m_rpLineVertexes = new osg::Vec3Array;
		m_rpLineVertexes->push_back(osg::Vec3(0,0,0));
		m_rpLineVertexes->push_back(osg::Vec3(0,0,0));
		m_rpGeomLinkLine->setVertexArray(m_rpLineVertexes);
		m_rpGeomLinkLine->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, m_rpLineVertexes->size()));

		osg::ref_ptr<osg::Vec3Array> vecNorm = new osg::Vec3Array;
		vecNorm->push_back(osg::Vec3(0, 0, 1));
		vecNorm->at(0).normalize();
		m_rpGeomLinkLine->setNormalArray(vecNorm, osg::Array::BIND_OVERALL);

		osg::ref_ptr<osg::Vec4Array> vecColor = new osg::Vec4Array;
		vecColor->push_back(osg::Vec4(1, 1, 0, 1));
		m_rpGeomLinkLine->setColorArray(vecColor, osg::Array::BIND_OVERALL);
		m_rpGeomLinkLine->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	}

	void CExLabelNode::CreateTitle()
	{
		m_rpTitleNode = new CExLabelItemNode(m_opRenderContext.get(), GetOption());
		m_rpCameraNode->addChild(m_rpTitleNode);
		m_listItemNode.push_back(m_rpTitleNode);
	}

	void CExLabelNode::InitPosition()
	{
		const double fViewWidth = m_rpTitleNode->GetCurrentViewPortWidth();
		const double fViewHeight = m_rpTitleNode->GetCurrentViewPortHeight();

		if(GetOption()->BindingLLH().valid())
		{
			m_bEnableBinding = true;
		}

		/// 根据绑定点计算并设置标牌起始位置
		osg::Vec2d vecLT(0.5, 0.5);

		if(m_bEnableBinding)
		{
			osg::Vec3d vecXYZ;
			
			if( FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), GetOption()->BindingLLH(), vecXYZ)
				&& FeUtil::ConvertLocalWorldCoordToScreen(m_opRenderContext.get(), vecXYZ, vecLT))
			{
				vecLT.x() /= fViewWidth;
				vecLT.y() /= fViewHeight;
				m_vecBindPtNorm = vecLT;   // 记录绑定点屏幕归一化坐标

				const double heightNorm = GetOption()->Height() / fViewHeight;
				const double fDistance = 10.0; // 绑定点距label的距离

				vecLT += osg::Vec2d(fDistance/fViewWidth , fDistance/fViewHeight + heightNorm) ;
			}
		}

		SetLabelPosition(vecLT);


		/// 连接线

		if(m_listItemNode.size() >= 1)
		{
			m_vecLineStartPtNorm = vecLT;
		}

		UpdateBindingLine();
	}

	void CExLabelNode::SetLabelPosition( const osg::Vec2& vecLTNorm )
	{
// 		if(vecLTNorm == m_rpTitleNode->GetPosition())
// 		{
// 			return;
// 		}

		/// 设置位置
		osg::Vec2d vecLT = vecLTNorm;
		const double fViewHeight = m_rpTitleNode->GetCurrentViewPortHeight();

        CExLabelItemNodeList::iterator it = m_listItemNode.begin();
        while(it != m_listItemNode.end())
        {
            CExLabelItemNode* pNode = *it;
            if(pNode)
            {
                pNode->SetPosition(vecLT);
                vecLT.y() -= (pNode->GetOption()->Height() / fViewHeight);
            }
			++it;
        }
	}

	void CExLabelNode::UpdateLinkTypeByBindingPoint()
	{
		/// 找出离绑定点最近的顶点，作为连接线起始点
		const float fViewWidth = m_rpTitleNode->GetCurrentViewPortWidth();
		const float fViewHeight = m_rpTitleNode->GetCurrentViewPortHeight();

		/// 当前标牌四个顶点
		osg::Vec2 vecLT = m_rpTitleNode->GetPosition();
		osg::Vec2 vecRT = vecLT + osg::Vec2(m_rpTitleNode->GetWidth()/fViewWidth, 0);
		osg::Vec2 vecLB = m_listItemNode.back()->GetPosition() + osg::Vec2(0, -m_listItemNode.back()->GetHeight()/fViewHeight);
		osg::Vec2 vecRB = vecLB + osg::Vec2(m_listItemNode.back()->GetWidth()/fViewWidth, 0);

		/// 绑定点距离四个顶点的距离
		double lt = (m_vecBindPtNorm.x()-vecLT.x())*(m_vecBindPtNorm.x()-vecLT.x()) 
			+ (m_vecBindPtNorm.y()-vecLT.y()) * (m_vecBindPtNorm.y()-vecLT.y()) ;

		double lb = (m_vecBindPtNorm.x()-vecLB.x())*(m_vecBindPtNorm.x()-vecLB.x()) 
			+ (m_vecBindPtNorm.y()-vecLB.y()) * (m_vecBindPtNorm.y()-vecLB.y()) ;

		double rt = (m_vecBindPtNorm.x()-vecRT.x())*(m_vecBindPtNorm.x()-vecRT.x()) 
			+ (m_vecBindPtNorm.y()-vecRT.y()) * (m_vecBindPtNorm.y()-vecRT.y()) ;

		double rb = (m_vecBindPtNorm.x()-vecRB.x())*(m_vecBindPtNorm.x()-vecRB.x()) 
			+ (m_vecBindPtNorm.y()-vecRB.y()) * (m_vecBindPtNorm.y()-vecRB.y()) ;

		/// 找出距离最小点，设置连接类型
		double minL = lt;
		m_eLinkType = E_LINK_LT;

		if(lb < minL)
		{
			minL = lb;
			m_eLinkType = E_LINK_LB;
		}
		if(rt < minL)
		{
			minL = rt;
			m_eLinkType = E_LINK_RT;
		}
		if(rb < minL)
		{
			m_eLinkType = E_LINK_RB;
		}
	}

	void CExLabelNode::Redraw()
	{
		UpdateLabelByBindingPoint();
		UpdateBindingLine();
		m_bNeedRedraw = false;
	}

	void CExLabelNode::UpdateBindingLine()
	{
		if(!m_bEnableBinding)
		{
			m_rpGeomLinkLine->setNodeMask(UN_VISIBLE_EVENT_MASK);
			return;
		}

		m_rpGeomLinkLine->setNodeMask(VISIBLE_MASK);

		/// 根据当前连接方式计算连接线起始点
		const float fViewWidth = m_rpTitleNode->GetCurrentViewPortWidth();
		const float fViewHeight = m_rpTitleNode->GetCurrentViewPortHeight();

		CExLabelItemNode* linkNode = m_rpTitleNode;

		if(E_LINK_LB == m_eLinkType)
		{
			if(m_bShowItem)
			{
				linkNode = m_listItemNode.back();
			}

			m_vecLineStartPtNorm = linkNode->GetPosition() 
				+ osg::Vec2(0, -linkNode->GetHeight()/fViewHeight);
		}
		else if(E_LINK_RT == m_eLinkType)
		{
			m_vecLineStartPtNorm = m_rpTitleNode->GetPosition() 
				+ osg::Vec2(m_rpTitleNode->GetWidth()/fViewWidth, 0);
		}
		else if(E_LINK_RB == m_eLinkType)
		{
			if(m_bShowItem)
			{
				linkNode = m_listItemNode.back();
			}

			m_vecLineStartPtNorm = linkNode->GetPosition() 
				+ osg::Vec2(linkNode->GetWidth()/fViewWidth, -linkNode->GetHeight()/fViewHeight);
		}
		else // LT
		{
			m_vecLineStartPtNorm = m_rpTitleNode->GetPosition();
		}
	
		/// 更新连接线顶点
		if(m_rpLineVertexes->size() >= 2)
		{
			if(m_rpLineVertexes->at(0) != osg::Vec3(m_vecLineStartPtNorm, 0)
				|| m_rpLineVertexes->at(1) != osg::Vec3(m_vecBindPtNorm, 0))
			{
				m_rpLineVertexes->at(0) = osg::Vec3(m_vecLineStartPtNorm, 0);
				m_rpLineVertexes->at(1) = osg::Vec3(m_vecBindPtNorm, 0);
				m_rpGeomLinkLine->dirtyDisplayList();
				m_rpGeomLinkLine->dirtyBound();
			}
		}
	}

	void CExLabelNode::UpdateLabelByBindingPoint()
	{
		/// 无绑定，则不更新位置
		if(!m_bEnableBinding)
		{
			return;
		}

		/// 根据绑定点计算位置
		osg::Vec3d vecXYZ;
		osg::Vec2d vecBindingPt(0.5, 0.5);
		osg::Vec2d vecLabelPos(0.5, 0.5);

		if( FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), GetOption()->BindingLLH(), vecXYZ)
			&& FeUtil::ConvertLocalWorldCoordToScreen(m_opRenderContext.get(), vecXYZ, vecBindingPt))
		{
			const double fViewWidth = m_rpTitleNode->GetCurrentViewPortWidth();
			const double fViewHeight = m_rpTitleNode->GetCurrentViewPortHeight();

			vecBindingPt.x() /= fViewWidth;
			vecBindingPt.y() /= fViewHeight;

			if(m_bLineChanged)
			{
				/// 连接线与标牌间的距离发生变化，使用相对位移计算

				vecLabelPos = m_rpTitleNode->GetPosition() + (vecBindingPt - m_vecBindPtNorm);
			}
			else
			{
				const float fDistance = 10.0; // 绑定点距label的距离,默认状态
				
				vecLabelPos = vecBindingPt + osg::Vec2d(fDistance/fViewWidth , 
					fDistance/fViewHeight + GetOption()->Height() / fViewHeight);
			}

			m_vecBindPtNorm = vecBindingPt;

		}
		
		/// 检验位置
		MakePosValid(vecLabelPos);

		/// 更新标牌位置
		SetLabelPosition(vecLabelPos);

	}

	void CExLabelNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	std::string CExLabelNode::GetText()
	{
		return GetOption()->Text();
	}

	void CExLabelNode::SetText( const std::string& strTitle )
	{
		if(GetOption()->Text() != strTitle)
		{
			GetOption()->Text() = strTitle;

			m_rpTitleNode->m_text->setText(strTitle, osgText::String::ENCODING_UTF8);

		}
	}

	const osg::Vec4& CExLabelNode::GetTextColor()
	{
		return GetOption()->TextColor();
	}

	void CExLabelNode::SetTextColor( const osg::Vec4& vecColor )
	{
		if(GetOption()->TextColor() != vecColor)
		{
			GetOption()->TextColor() = vecColor;

			m_rpTitleNode->m_text->setColor(vecColor);
		}
	}

	const osg::Vec4& CExLabelNode::GetBackgroundColor()
	{
		return GetOption()->BackgroundColor();
	}

	void CExLabelNode::SetBackgroundColor( const osg::Vec4& vecColor )
	{
		if(GetOption()->BackgroundColor() != vecColor)
		{
			GetOption()->BackgroundColor() = vecColor;

			if(m_rpTitleNode->m_rpVecBkColor->size() >= 1)
			{
				m_rpTitleNode->m_rpVecBkColor->at(0) = vecColor;
				m_rpTitleNode->m_rpGeomQuad->dirtyDisplayList();
				m_rpTitleNode->m_rpGeomQuad->dirtyBound();
			}
		}
	}

	void CExLabelNode::SetBackgroundImage( const std::string& strPath )
	{

		osg::ref_ptr<osg::Image> rpImage = osgDB::readImageFile(strPath);
		GetOption()->BackgroundImagePath() = strPath;
		SetBackgroundImage(rpImage);
	}

	void CExLabelNode::SetBackgroundImage( osg::Image* pImage )
	{
		if(pImage)
		{
			GetOption()->BackgroundImage() = pImage;

			m_rpTitleNode->m_rpBkImageTexture->setImage(pImage);
		}
	}

	std::string CExLabelNode::GetBackgroundImage()
	{
		return GetOption()->BackgroundImagePath();
	}

	float CExLabelNode::GetFontSize()
	{
		return GetOption()->FontSize();
	}

	void CExLabelNode::SetFontSize( const float& fSize )
	{
		if(GetOption()->FontSize() != fSize)
		{
			GetOption()->FontSize() = fSize;

			m_rpTitleNode->m_text->setCharacterSize(fSize / m_rpTitleNode->GetCurrentViewPortHeight());
		}
	}

	const std::string& CExLabelNode::GetFontName()
	{
		return GetOption()->FontName();
	}

	void CExLabelNode::SetFontName( const std::string& fontName )
	{

	}

	float CExLabelNode::GetWidth()
	{
		return GetOption()->Width();
	}

	void CExLabelNode::SetWidth( const float& fWidth )
	{
		if(GetOption()->Width() != fWidth)
		{
			GetOption()->Width() = fWidth;

			m_rpTitleNode->UpdatePosition();
		}
	}

	float CExLabelNode::GetHeight()
	{
		return GetOption()->Height();
	}

	void CExLabelNode::SetHeight( const float& fHeight )
	{
		if(GetOption()->Height() != fHeight)
		{
			GetOption()->Height() = fHeight;

			m_rpTitleNode->UpdatePosition();
		}
	}

	CExLabelItemOption::E_TEXT_ALIGNMENT CExLabelNode::GetTextAlign()
	{
		return GetOption()->Alignment();
	}

	void CExLabelNode::SetTextAlign( CExLabelItemOption::E_TEXT_ALIGNMENT align )
	{
		if(align != GetOption()->Alignment())
		{
			GetOption()->Alignment() = align;

			m_rpTitleNode->UpdateTextAndViewPort();
		}
	}

	FeExtNode::CExLabelItemNodeList CExLabelNode::GetChildList()
	{
		return m_listItemNode;
	}

	void CExLabelNode::SetVisible( const bool& bVisible )
	{
		if(bVisible)
		{
			// 解决标牌隐藏后序列化，标头不显示问题
			if(m_rpTitleNode.valid())
			{
				m_rpTitleNode->SetVisible(bVisible);
			}

			m_bNeedRedraw = true;
		}

		CExternNode::SetVisible(bVisible);
	}

	void CExLabelNode::RemoveBackgroundImage()
	{
		GetOption()->BackgroundImagePath() = "";
		GetOption()->BackgroundImage() = NULL;
		m_rpTitleNode->m_rpGeomQuad->getOrCreateStateSet()->removeTextureAttribute(0, osg::StateAttribute::TEXTURE);
		m_rpTitleNode->m_rpBkImageTexture = new osg::Texture2D;
		m_rpTitleNode->m_rpGeomQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0 ,m_rpTitleNode->m_rpBkImageTexture, osg::StateAttribute::ON);
	}

}