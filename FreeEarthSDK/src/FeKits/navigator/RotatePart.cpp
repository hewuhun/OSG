#include "RotatePart.h"

#include <FeUtils/MathUtil.h>

namespace FeKit
{
	CRotatePart::CRotatePart
		( double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes )
		:CCompositPart(dLeft, dRight, dBottom, dTop, strRes)
	{
		double dPLeft = m_dLeft + m_dWidth * 0.1;
		double dPRight = m_dRight - m_dWidth * 0.1;
		double dPBotoom = m_dBottom + m_dHeight * 0.1;
		double dPTop = m_dTop - m_dHeight * 0.1;

		CRotatePanel* pPanel = new CRotatePanel(dPLeft, dPRight, dPBotoom, dPTop, strRes);
		addChild(pPanel);
		m_vecParts.push_back(pPanel);

		CRotateEdge* pEdge = new CRotateEdge(dPLeft, dPRight, dPBotoom, dPTop, strRes);
		addChild(pEdge);
		m_vecParts.push_back(pEdge);
	}

	CRotatePanel::CRotatePanel
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
		,m_dAzimuth(0.0)
		,m_dPitch(0.0)
	{
		m_dRate = 0.005;
		m_strImageName = strRes + "/DirectDiskRotate.png";
		Create();
	}


	CRotatePanel::~CRotatePanel(void)
	{

	}

	bool CRotatePanel::PushHandle( int nX, int nY )
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		if(!m_bSelect)
		{
			Select(true);
			UpdateRotate(nX, nY);
			return true;
		}

		return false;
	}

	bool CRotatePanel::DragHandle( int nX, int nY )
	{
		if(!m_bActive || !m_bSelect)
		{
			return false;
		}

		UpdateRotate(nX, nY);
		return true;
	}

	bool CRotatePanel::UpdateRotate( int nX, int nY )
	{
		GetDirction(nX, nY, m_dAzimuth, m_dPitch);

		if(m_opCtrl.valid())
		{
			m_opCtrl->UpdateRotate(m_dAzimuth, m_dPitch);
		}

		return true;
	}

	bool CRotatePanel::ReleaseHandle( int nX, int nY )
	{
		if(m_bSelect)
		{
			Select(false);

			if(m_opCtrl.valid())
			{
				m_opCtrl->StopUpdateRotate();
			}
		}

		return false;
	}

	void CRotatePanel::GetDirction( int nX, int nY, double& dAzimuth, double& dPitch )
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

		double dX = m_nCenterX + m_dRadius * cos(dCita);
		double dY = m_nCenterY + m_dRadius * sin(dCita);

		dAzimuth = (dX - m_nCenterX) / m_dRadius * m_dRate;
		dPitch = (dY - m_nCenterY) / m_dRadius * m_dRate;
	}

	bool CRotatePanel::DoubleClickHandle(int nX, int nY)
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		return true;
	}

	CRotateEdge::CRotateEdge(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
	{
		m_dNormalTran = 0.4;
		m_strImageName = strRes + "/DirectDiskEdge.png";

		Create();
	}

	bool CRotateEdge::WithinMe( int nX, int nY )
	{
		return false;
	}
}

