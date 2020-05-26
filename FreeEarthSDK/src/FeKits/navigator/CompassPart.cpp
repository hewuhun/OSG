#include <osgGA/EventVisitor>
#include <FeUtils/MathUtil.h>

#include "CompassPart.h"

namespace FeKit
{
	CCompassPart::CCompassPart
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CCompositPart(dLeft, dRight, dBottom, dTop, strRes)
		,m_opPlate(NULL)
		,m_opNeedle(NULL)
		,m_dNorthDegree(0.0)
	{
		double dNLeft = m_dLeft + m_dWidth * 0.15;
		double dNRight = m_dRight - m_dWidth * 0.15;
		double dNBotoom = m_dBottom + m_dHeight * 0.15;
		double dNTop = m_dTop - m_dHeight * 0.15;

		m_opNeedle = new CCompassNeedle(this,dNLeft, dNRight, dNBotoom, dNTop, strRes);
		addChild(m_opNeedle.get());
		m_vecParts.push_back(m_opNeedle.get());

		double dPLeft = m_dLeft + m_dWidth * 0.02;
		double dPRight = m_dRight - m_dWidth * 0.02;
		double dPBotoom = m_dBottom + m_dHeight * 0.02;
		double dPTop = m_dTop - m_dHeight * 0.02;
		m_opPlate = new CCompassPlate(dPLeft, dPRight, dPBotoom, dPTop, strRes);
		addChild(m_opPlate.get());
		m_vecParts.push_back(m_opPlate.get());
	}

	CCompassPart::~CCompassPart(void)
	{

	}

	void CCompassPart::UpdateValue( double dValue )
	{
		if(FeMath::Equal(dValue, m_dNorthDegree))
		{
			return;
		}

		bool bDirect = true; //true 逆时针, false 顺时针
		if(dValue > 0.0)
		{
			bDirect = true;
		}
		else if(dValue < 0.0)
		{
			bDirect = false;
		}
		else
		{
			return;
		}

		double dDeltaX = m_dWidth * 0.5 + m_dLeft;
		double dDeltaY = m_dHeight * 0.5 + m_dBottom;
		osg::Matrix tM = osg::Matrix::translate(0.0 - dDeltaX, 0.0 - dDeltaY, -1.0);
		osg::Matrix itM = osg::Matrix::translate(dDeltaX, dDeltaY, -1.0);
		osg::Matrix rM = osg::Matrix::rotate(dValue,  osg::Z_AXIS );

		if(m_opNeedle.valid())
		{
			m_opNeedle->UpdateValue(dValue);
			m_opNeedle->setMatrix(tM * rM * itM);
		}

		if(m_opPlate.valid())
		{
			m_opPlate->setMatrix(tM * rM * itM);
		}

		m_dNorthDegree = dValue;
	}

	bool CCompassPart::WithinMe(int nX, int nY)
	{
		return false;
	}

	void CCompassPart::SetCtrl( CNavigatorCtrl* pCtrl )
	{
		CCompositPart::SetCtrl(pCtrl);

		pCtrl->AddActiveComponent(this);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	CCompassNeedle::CCompassNeedle(
		CCompassPart* pParent,
		double dLeft, 
		double dRight, 
		double dBottom, 
		double dTop, 
		const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
		,m_dNorthDegree(0.0)
		,m_opParent(pParent)
	{
		m_nPrePosX = 0;
		m_nPrePosY = 0;

		m_strImageName = strRes + "/Compass.png";
		Create();
	}

	void CCompassNeedle::UpdateValue( double dValue )
	{
		m_dNorthDegree = dValue;
	}

	bool CCompassNeedle::PushHandle(int nX, int nY)
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		Select(true);

		m_nPrePosX = nX;
		m_nPrePosY = nY;

		return true;
	}

	bool CCompassNeedle::DragHandle(int nX, int nY)
	{
		if(!m_bActive || !m_bSelect)
		{
			return false;
		}

		SetCurrentDirection(GetDeltaNorthDegree(nX, nY));

		m_nPrePosX = nX;
		m_nPrePosY = nY;

		return true;
	}

	bool CCompassNeedle::ReleaseHandle(int nX, int nY)
	{
		if(m_bSelect)
		{
			Select(false);
		}

		return false;
	}

	bool CCompassNeedle::DoubleClickHandle( int nX, int nY )
	{
		if(!m_bActive || !WithinMe(nX, nY)) 
		{
			return false;
		}

		if(m_opParent.valid())
		{
			m_opParent->UpdateValue(0.0);
		}

		if(m_opCtrl.valid())
		{
			m_opCtrl->UpdateHeading(0.0);
		}

		return true;
	}

	double CCompassNeedle::GetDeltaNorthDegree( int nX, int nY )
	{
		osg::Vec3d vecPre(m_nPrePosX - m_nCenterX, m_nPrePosY - m_nCenterY, 0.0);
		osg::Vec3d vecCurrent(nX - m_nCenterX, nY - m_nCenterY, 0.0);

		double dCita = acos((vecPre * vecCurrent) / (vecPre.length() * vecCurrent.length()));

		//dValue = dX1 * dY2 - dX2 * dY1;
		double dDirection = vecPre.x() * vecCurrent.y() - vecCurrent.x() * vecPre.y();
		if(dDirection > 0) return dCita; //顺时针
		else if(dDirection < 0) return -1.0 * dCita; //逆时针

		return 0.0;
	}

	void CCompassNeedle::SetCurrentDirection( double dValue )
	{
		bool bDirect = true; //true 逆时针, false 顺时针
		double currentDegree = 0.0;

		if(dValue > 0.0)
		{
			bDirect = true;
		}
		else if(dValue < 0.0)
		{
			bDirect = false;
		}
		else
		{
			return;
		}

		currentDegree = m_dNorthDegree + dValue;
		if(currentDegree >= PI__2)
		{
			currentDegree -= PI__2;
		}
		else if(currentDegree < 0.0)
		{
			currentDegree = PI__2 + currentDegree;
		}

		if(m_opCtrl.valid())
		{
			m_opCtrl->UpdateHeading(currentDegree);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	CCompassPlate::CCompassPlate
		(double dLeft, double dRight, double dBottom, double dTop, const std::string& strRes)
		:CLeafPart(dLeft, dRight, dBottom, dTop, strRes)
	{
		m_dNormalTran = 0.4;
		m_strImageName =  strRes + "/Compass_Edge.png";

		Create();
	}

	bool CCompassPlate::WithinMe( int nX, int nY )
	{
		return false;
	}
}
