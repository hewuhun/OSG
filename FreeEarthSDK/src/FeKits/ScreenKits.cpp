#include <FeKits/ScreenKits.h>

#include <osg/Geode>
#include <FeUtils/UtilityGeom.h>

namespace FeKit
{

    CScreenKits::CScreenKits(FeUtil::CRenderContext* pContext, int nXOffset, int nYOffset, int nWidth, int nHeight, CPlaceStrategy::KIT_PLACE ePlace )
        :osg::Camera()
        ,CKitsCtrl(pContext)
        ,m_nX(0)
        ,m_nY(0)
        ,m_nWidth(nWidth)
        ,m_nHeight(nHeight)
        ,m_dRightRate(-1.0)
        ,m_dLeftRate(1.0)
        ,m_dBottomRate(-1.0)
        ,m_dTopRate(1.0)
        ,m_dWidthRate(2.0)
        ,m_dHeightRate(2.0)
        ,m_dDepth(-1.0)
		,m_nOrderNum(30)
        ,m_rpPlaceStrgy(NULL)
    {
        m_rpPlaceStrgy = new CAbsoluteStrategy(nXOffset, nYOffset, m_nWidth, m_nHeight, ePlace);

        InitHUD();
        UpdteProjection();

		m_bShow ? Show() : Hide();
    }

    CScreenKits::~CScreenKits()
    {

    }

    void CScreenKits::UpdteProjection()
    {
        if(m_nHeight != 0)
        {
            m_dRightRate = (m_nWidth * 1.0) / m_nHeight;
            m_dLeftRate = m_dRightRate * -1.0;

            m_dWidthRate = m_dRightRate - m_dLeftRate;
            m_dHeightRate = m_dTopRate - m_dBottomRate;
        }

        setViewport(m_nX, m_nY, m_nWidth, m_nHeight);
		setProjectionMatrix(
			osg::Matrix::ortho(m_dLeftRate, m_dRightRate, m_dBottomRate, m_dTopRate, -10.0, 10.0));
    }

    void CScreenKits::SetPosition( int nX, int nY, int nWidth, int nHeight )
    {
        m_nX = nX;
        m_nY = nY;
        m_nWidth = nWidth;
        m_nHeight = nHeight;

        UpdteProjection();
    }

    void CScreenKits::GetPosition( int& nX, int& nY, int& nWidth, int& nHeight )
    {
        nX = m_nX;
        nY = m_nY;
        nWidth = m_nWidth;
        nHeight = m_nHeight;
    }


    void CScreenKits::GetOrthoRate( double& dLeft, double& dRight, double& dBottom, double& dTop ) const
    {
        dLeft = m_dLeftRate;
        dRight = m_dRightRate;
        dBottom = m_dBottomRate;
        dTop = m_dTopRate;
    }

    double CScreenKits::GetScreeDepth() const
    {
        return m_dDepth;
    }

    void CScreenKits::SetScreeDepth( double dDepth /*= -1.0*/ )
    {
        m_dDepth = dDepth;
    }

    void CScreenKits::Show()
    {
        m_bShow = true;
        setNodeMask(0xffffffff);
    }

    void CScreenKits::Hide()
    {
        m_bShow = false;
        setNodeMask(0x00000000);
    }

    bool CScreenKits::WithMe( int unX, int unY )
    {
        int unDeltaWidth = unX - m_nX;
        int unDeltaHeight = unY - m_nY;

        if( (unDeltaWidth >= 0 && unDeltaWidth <= m_nWidth ) &&
            (unDeltaHeight >= 0 && unDeltaHeight <= m_nHeight))
        {
            return true;
        }

        return false;
    }

    void CScreenKits::ResizeKit( int nW, int nH )
    {
        if(m_rpPlaceStrgy.valid())
        {
            int nX = 0;
            int nY = 0;
            m_rpPlaceStrgy->GetXY(nW, nH, nX, nY);
            SetPosition(nX, nY, m_nWidth, m_nHeight);
        }
    }

    void CScreenKits::InitHUD()
    {
        setViewMatrix(osg::Matrix::identity());
        setRenderOrder(osg::Camera::POST_RENDER, m_nOrderNum);
        setClearMask(GL_DEPTH_BUFFER_BIT);
        setAllowEventFocus(true);
        setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    }

}

namespace FeKit
{

    CAbsoluteStrategy::CAbsoluteStrategy( 
        int nXOffset, 
        int nYOffset, 
        int nWidth, 
        int nHeight, 
        KIT_PLACE ePlace )
        :CPlaceStrategy()
        ,m_ePlace(ePlace)
        ,m_nXOffset(nXOffset)
        ,m_nYOffset(nYOffset)
        ,m_nWidth(nWidth)
        ,m_nHeight(nHeight)
    {

    }

    CAbsoluteStrategy::~CAbsoluteStrategy()
    {

    }

    void CAbsoluteStrategy::GetXY( int nScreenW, int nScreenH, int& nX, int& nY )
    {
        switch(m_ePlace)
        {
        case LEFT_TOP:
            nX = m_nXOffset;
            nY = nScreenH - (m_nYOffset + m_nHeight);
            break;

        case LEFT_BOTTOM:
            nX = m_nXOffset;
            nY = m_nYOffset;
            break;

        case RIGHT_TOP:
            nX = nScreenW - (m_nWidth + m_nXOffset);
            nY = nScreenH - (m_nYOffset + m_nHeight);
            break;

        case RIGHT_BOTTOM:
            nX = nScreenW - (m_nWidth + m_nXOffset);
            nY = m_nYOffset;
            break;
        }
    }

}
