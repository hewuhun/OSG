#include "NavigatorPart.h"

#include "CompassPart.h"
#include "RotatePart.h"
#include "MovePart.h"
#include "ZoomPart.h"

namespace FeKit
{

    CNavigatorPart::CNavigatorPart
        (double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
        :CCompositPart(dLeft, dRight, dBottom, dTop, strRes)
        ,m_opCompass(NULL)
        ,m_strRes(strRes)
    {
        CreateCompassPart();

        CreateRotatePart();

        CreateMovePart();

        CreateZoomPart();
    }

    CNavigatorPart::~CNavigatorPart(void)
    {
    }

    bool CNavigatorPart::CreateCompassPart()
    {
        //Create 指南针针盘
        double dLeft = m_dLeft;
        double dRight = m_dRight;
        double dBottom = m_dBottom + m_dHeight * 0.75;
        double dTop = m_dTop;
        m_opCompass = new CCompassPart(dLeft, dRight, dBottom, dTop, m_strRes);
        addChild(m_opCompass.get());
        m_vecParts.push_back(m_opCompass.get());

        return true;
    }


    bool CNavigatorPart::CreateRotatePart()
    {
        double dLeft = m_dLeft;
        double dRight = m_dRight;
        double dBottom = m_dBottom + (m_dHeight * 9.0 / 16);
        double dTop = m_dTop - (0.25 * m_dHeight);

        CRotatePart* pRotatePart = new CRotatePart(dLeft, dRight, dBottom, dTop, m_strRes);
        addChild(pRotatePart);
        m_vecParts.push_back(pRotatePart);
        return true;
    }

    bool CNavigatorPart::CreateMovePart()
    {
        double dLeft = m_dLeft;
        double dRight = m_dRight;
        double dBottom = m_dBottom;
        double dTop = m_dTop - (m_dHeight * 13.0 / 16.0);
        
        CMovePart* pMovePart = new CMovePart(dLeft, dRight, dBottom, dTop, m_strRes);
        addChild(pMovePart);
        m_vecParts.push_back(pMovePart);

        return true;
    }

    bool CNavigatorPart::CreateZoomPart()
    {
        //创建操作地球旋转按键

        double dLeft = m_dLeft;
        double dRight = m_dRight;
        double dBottom = m_dBottom + (m_dHeight * 3.0 / 16.0);
        double dTop = m_dTop - (m_dHeight * 7.0 / 16.0);

        CZoomPart* pZoomPart = new CZoomPart(dLeft, dRight, dBottom, dTop, m_strRes);
        addChild(pZoomPart);
        m_vecParts.push_back(pZoomPart);
        return true;
    }


    bool CNavigatorPart::WithinMe( int nX, int nY )
    {
        if((nX >= m_nPosX) && 
           (nX <= (m_nPosX + m_nWidth)) &&
           (nY >= m_nPosY) &&
           (nY <= (m_nPosY + m_nHeight)))
        {
            return true;
        }

        return false;
    }

    void CNavigatorPart::UpdateValue( double dValue )
    {
        if(m_opCompass.valid())
        {
            m_opCompass->UpdateValue(dValue);
        }
    }

    bool CNavigatorPart::MoveHandle( int nX, int nY )
    {
        if(WithinMe(nX, nY))
        {
            if(!m_bActive)
            {
                m_bActive = true;
                Active(m_bActive);
                return true;
            }
        }
        else
        {
            if(m_bActive)
            {
                m_bActive = false;
                Active(m_bActive);
            }
        }

        return false;
    }

    void CNavigatorPart::ShowOnlyCompass( bool bShow )
    {
        osg::Node::NodeMask nodeMask;
        if(bShow)
        {
            nodeMask = 0x00000000;
        }
        else
        {
            nodeMask = 0xffffffff;
        }

        ChildPart::iterator itr = m_vecParts.begin();
        for(; itr != m_vecParts.end(); ++itr)
        {
            if(itr->valid() && itr->get() != m_opCompass.get())
            {
                itr->get()->setNodeMask(nodeMask);
            }
        }
    }


}

