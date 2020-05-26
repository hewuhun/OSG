#include "MovePart.h"

namespace FeKit
{
    CMovePart::CMovePart
        ( double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes )
        :CCompositPart(dLeft, dRight, dBottom, dTop, strRes)
    {
        double dPLeft = m_dLeft + m_dWidth * 0.1;
        double dPRight = m_dRight - m_dWidth * 0.1;
        double dPBotoom = m_dBottom + m_dHeight * 0.1;
        double dPTop = m_dTop - m_dHeight * 0.1;

        CMovePanel* pPanel = new CMovePanel(dPLeft, dPRight, dPBotoom, dPTop, strRes);
        addChild(pPanel);
        m_vecParts.push_back(pPanel);

        CMoveEdge* pEdge = new CMoveEdge(dPLeft, dPRight, dPBotoom, dPTop, strRes);
        addChild(pEdge);
        m_vecParts.push_back(pEdge);
    }
	
	CMovePart::~CMovePart( void )
	{

	}

	///////////////////////////////////////////////////////////////////////////
    CMoveEdge::CMoveEdge
        (double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
        :CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
    {
        m_dNormalTran = 0.4;
        m_strImageName = strRes + "/DirectDiskEdge.png";

        Create();
	}

	CMoveEdge::~CMoveEdge()
	{

	}
	
    bool CMoveEdge::WithinMe( int nX, int nY )
    {
        return false;
    }

	//////////////////////////////////////////////////////////////////////////
	CMovePanel::CMovePanel
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
		,m_dY(0.0)
		,m_dX(0.0)
	{
		m_dRate = 0.005;
		m_strImageName = strRes + "/DirectDiskMove.png";
		Create();
	}

	CMovePanel::~CMovePanel( void )
	{

	}

    bool CMovePanel::PushHandle( int nX, int nY )
    {
        if(!m_bActive || !WithinMe(nX, nY)) 
        {
            return false;
        }

        if(!m_bSelect)
        {
            Select(true);
            UpdateMove(nX, nY);
            return true;
        }

        return false;
    }

    bool CMovePanel::DragHandle( int nX, int nY )
    {
        if(!m_bActive || !m_bSelect)
        {
            return false;
        }

        UpdateMove(nX, nY);
        return true;
    }

    bool CMovePanel::ReleaseHandle( int nX, int nY )
    {
        if(m_bSelect)
        {
            Select(false);

            if(m_opCtrl.valid())
            {
                m_opCtrl->StopUpdatePan();
            }
        }

        return false;
    }

    void CMovePanel::GetDirction( int nX, int nY, double& dX, double& dY )
    {
        osg::Vec3d vecPre(
            (m_nPosX + m_nWidth) - m_nCenterX, 
            m_nPosY + m_dRadius - m_nCenterY, 
            0.0);
        osg::Vec3d vecCurrent(nX - m_nCenterX, nY - m_nCenterY, 0.0);

        double dCita = acos((vecPre * vecCurrent) / (vecPre.length() * vecCurrent.length()));

        //dValue = dX1 * dY2 - dX2 * dY1;
        double dDirection = vecPre.x() * vecCurrent.y() - vecCurrent.x() * vecPre.y();
        if(dDirection < 0) //顺时针
        {
            dCita = PI__2 - dCita;
        }

        double dTempX = m_nCenterX + m_dRadius * cos(dCita);
        double dTempY = m_nCenterY + m_dRadius * sin(dCita);

        dX = (dTempX - m_nCenterX) / m_dRadius * m_dRate;
        dY = (dTempY - m_nCenterY) / m_dRadius * m_dRate;
    }

    void CMovePanel::UpdateMove( int nX, int nY )
    {
        GetDirction(nX, nY, m_dX, m_dY);

        if(m_opCtrl.valid())
        {
            m_opCtrl->UpdatePan(m_dX, m_dY);
        }
    }

	bool CMovePanel::DoubleClickHandle(int nX, int nY)
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		return true;
	}
}

