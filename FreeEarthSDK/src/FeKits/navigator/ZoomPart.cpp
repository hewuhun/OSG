#include "ZoomPart.h"

namespace FeKit
{
	CZoomPart::CZoomPart
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CCompositPart(dLeft, dRight, dBottom, dTop, strRes)
	{
		double dIBtnLeft = m_dLeft + 0.4 * m_dWidth;
		double dIBtnRight = m_dRight - 0.4 * m_dWidth;
		double dIBtnBottom = m_dTop - 0.1 * m_dHeight;
		double dIBtnTop = m_dTop;
		CZoomInButton* pInButton = new CZoomInButton(dIBtnLeft, dIBtnRight, dIBtnBottom, dIBtnTop, strRes);
		addChild(pInButton);
		m_vecParts.push_back(pInButton);

		double dOBtnLeft = m_dLeft + 0.4 * m_dWidth;
		double dOBtnRight = m_dRight - 0.4 * m_dWidth;
		double dOBtnBottom = m_dBottom;
		double dOBtnTop = m_dBottom + m_dHeight * 0.1;
		CZoomOutButton* pOutButton = new CZoomOutButton(dOBtnLeft, dOBtnRight, dOBtnBottom, dOBtnTop, strRes);
		addChild(pOutButton);
		m_vecParts.push_back(pOutButton);

		double dSBlkLeft = m_dLeft + 0.4 * m_dWidth;
		double dSBlkRight = m_dRight - 0.4 * m_dWidth;
		double dSBlkBottom = m_dBottom + (10.0 / 21 * m_dHeight);
		double dSBlkTop = m_dTop - (10.0 / 21 * m_dHeight);
		CZoomSlidBlock* pSlidBlock = new CZoomSlidBlock(dSBlkLeft, dSBlkRight, dSBlkBottom, dSBlkTop, strRes);
		pSlidBlock->SetRange(0.7);
		addChild(pSlidBlock);
		m_vecParts.push_back(pSlidBlock);

		double dSPolLeft = m_dLeft + (7.0 / 15.0 * m_dWidth);
		double dSPolRight = m_dRight - (7.0 / 15.0 * m_dWidth);
		double dSPolBottom = m_dBottom + m_dHeight * 0.1;
		double dSPolTop = m_dTop - 0.1 * m_dHeight;
		CZoomPole* pPole = new CZoomPole(dSPolLeft, dSPolRight, dSPolBottom, dSPolTop, strRes);
		addChild(pPole);
		m_vecParts.push_back(pPole);  

		double dZEgeLeft = m_dLeft + 0.4 * m_dWidth;
		double dEgeRight = m_dRight - 0.4 * m_dWidth;
		double dEgeBottom = m_dBottom;
		double dEgeTop = m_dTop;
		CZoomEdge* pEdge = new CZoomEdge(dZEgeLeft, dEgeRight, dEgeBottom, dEgeTop, strRes);
		addChild(pEdge);
		m_vecParts.push_back(pEdge);  
	}

	CZoomPart::~CZoomPart(void)
	{

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	CZoomPole::CZoomPole(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
	{
		m_strImageName = strRes + "/ZomPole.png";
		Create(false);
	}

	bool CZoomPole::WithinMe( int nX, int nY )
	{
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	CZoomInButton::CZoomInButton
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
	{
		m_strImageName = strRes + "/ZoomIn.png";
		Create(false);
	}

	bool CZoomInButton::PushHandle( int nX, int nY )
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		if(!m_bSelect)
		{
			Select(true);

			if(m_opCtrl.valid())
			{
				m_opCtrl->UpdateZoom(-0.01);
			}

			return true;
		}

		return false;
	}

	bool CZoomInButton::ReleaseHandle( int nX, int nY )
	{
		if(m_bSelect)
		{
			Select(false);

			if(m_opCtrl.valid())
			{
				m_opCtrl->StopUpdateZoom();
			}
		}

		return false;
	}

	bool CZoomInButton::DoubleClickHandle(int nX, int nY)
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	CZoomOutButton::CZoomOutButton
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
	{
		m_strImageName =  strRes + "/ZoomOut.png";
		Create(false);
	}

	bool CZoomOutButton::PushHandle( int nX, int nY )
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		if(!m_bSelect)
		{
			Select(true);

			if(m_opCtrl.valid())
			{
				m_opCtrl->UpdateZoom(0.01);
			}

			return true;
		}

		return false;
	}

	bool CZoomOutButton::ReleaseHandle( int nX, int nY )
	{
		if(m_bSelect)
		{
			Select(false);

			if(m_opCtrl.valid())
			{
				m_opCtrl->StopUpdateZoom();
			}
		}

		return false;
	}

	bool CZoomOutButton::DoubleClickHandle(int nX, int nY)
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	CZoomSlidBlock::CZoomSlidBlock
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
		,m_dRangeRate(0.0)
		,m_nRange(0)
		,m_dRate(0.0)
		,m_eZoomState(NOON)
	{
		m_strImageName = strRes + "/ZoomSlider.png";
		Create(false);
	}

	bool CZoomSlidBlock::ReleaseHandle( int nX, int nY )
	{
		if(m_bSelect)
		{
			Select(false);

			if(m_opCtrl.valid())
			{
				m_opCtrl->StopUpdateZoom();
				m_eZoomState = NOON;
			}
		}

		SetCurrentPos();

		return false;
	}

	bool CZoomSlidBlock::DragHandle( int nX, int nY )
	{
		if(!m_bActive || !m_bSelect)
		{
			return false;
		}

		GetDeltaValue(nX, nY, m_eZoomState, m_dRate);

		if(m_opCtrl.valid() )
		{
			if(m_eZoomState == ZOOM_IN)
			{
				m_opCtrl->UpdateZoom(m_dRate * -0.1);
			}
			else if(m_eZoomState == ZOOM_OUT)
			{
				m_opCtrl->UpdateZoom(m_dRate * 0.1);
			}
		}

		SetCurrentPos();

		return true;
	}

	bool CZoomSlidBlock::PushHandle( int nX, int nY )
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		if(!m_bSelect)
		{
			Select(true);
			return true;
		}

		return false;
	}

	void CZoomSlidBlock::GetDeltaValue( int nX, int nY, EZoomState& eState, double& dRate )
	{

		double dDeltaValue = nY - m_nCenterY;
		if(dDeltaValue > 0)
		{
			eState = ZOOM_IN;
		}
		else if(dDeltaValue < 0)
		{
			eState = ZOOM_OUT;
			dDeltaValue *= -1;
		}
		else
		{
			eState = m_eZoomState;
		}

		if(dDeltaValue > m_nRange)
		{
			dDeltaValue = m_nRange;
		}

		dRate = dDeltaValue / m_nRange;
	}


	void CZoomSlidBlock::SetRange( double dRangeRate )
	{
		m_dRangeRate = dRangeRate;
	}


	void CZoomSlidBlock::UpdatePos( int nX, int nY, int nWidth, int nHeight, double dWR, double dHR )
	{
		CLeafPart::UpdatePos(nX, nY, nWidth, nHeight, dWR, dHR);

		m_nRange = nWidth * m_dRangeRate;
	}

	void CZoomSlidBlock::SetCurrentPos()
	{
		osg::Vec3d vecTrans(0.0, 0.0, -1.0);
		if(m_eZoomState == ZOOM_IN)
		{
			vecTrans.set(0.0, m_dRangeRate * 0.5 * m_dRate, -1.0);
		}
		else if(m_eZoomState == ZOOM_OUT)
		{
			vecTrans.set(0.0, m_dRangeRate * -0.5 * m_dRate, -1.0);
		}

		setMatrix(osg::Matrix::translate(vecTrans));
	}

	bool CZoomSlidBlock::DoubleClickHandle(int nX, int nY)
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		return true;
	}

	CZoomEdge::CZoomEdge
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
	{
		m_dNormalTran = 0.4;
		m_strImageName = strRes + "/ZoomEdge.png"; 
		Create(false);
	}
}
