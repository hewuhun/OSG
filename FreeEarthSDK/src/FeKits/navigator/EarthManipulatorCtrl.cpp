#include <osgEarth/Viewpoint>

#include <osgGA/KeySwitchMatrixManipulator>

#include <FeKits/navigator/EarthManipulatorCtrl.h>
#include <FeKits/navigator/BasePart.h>

#include <FeUtils/MathUtil.h>
#include <FeKits/manipulator/ManipulatorUtil.h>

namespace FeKit
{
	CEarthManipulatorCtrl::CEarthManipulatorCtrl(FeKit::FreeEarthManipulator* pManipulator)
		:CNavigatorCtrl()
		,m_bIsZoom(false)
		,m_dZoomRate(0.0)

		,m_bIsRotate(false)
		,m_dAzimuth(0.0)
		,m_dPitch(0.0)

		,m_bIsMove(false)
		,m_dX(0.0)
		,m_dY(0.0)

		,m_opFreeEarthManipulator(pManipulator)
		,m_opActiveCompass(NULL)
	{

	}


	CEarthManipulatorCtrl::~CEarthManipulatorCtrl(void)
	{
	}

	bool CEarthManipulatorCtrl::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
	{
		Update();

		return false;
	}

	void CEarthManipulatorCtrl::UpdateHeading( double dValue )
	{
		if(m_opFreeEarthManipulator.valid())
		{
			osgEarth::Viewpoint point = m_opFreeEarthManipulator->getViewpoint();
			point.setHeading(Navigator2Manipulator(dValue));
			m_opFreeEarthManipulator->setViewpoint(point);
		}
	}

	void CEarthManipulatorCtrl::StopUpdateHeading()
	{

	}

	void CEarthManipulatorCtrl::UpdatePan( double dX, double dY )
	{
		m_bIsMove = true;
		if(!FeMath::Equal(m_dX, dX) || !FeMath::Equal(m_dY, dY))
		{
			m_dX = dX;
			m_dY = dY;
		}
	}

	void CEarthManipulatorCtrl::StopUpdatePan()
	{
		m_bIsMove = false;
		m_dX = 0.0;
		m_dY = 0.0;
	}

	void CEarthManipulatorCtrl::UpdateRotate( double dAzimuth, double dPitch )
	{
		m_bIsRotate = true;
		if(!FeMath::Equal(m_dAzimuth, dAzimuth) || !FeMath::Equal(m_dPitch, dPitch))
		{
			m_dAzimuth = dAzimuth;
			m_dPitch = dPitch;
		}
	}

	void CEarthManipulatorCtrl::StopUpdateRotate()
	{
		m_bIsRotate = false;
		m_dAzimuth = 0.0;
		m_dPitch = 0.0;
	}

	void CEarthManipulatorCtrl::UpdateZoom( double dRate )
	{
		m_bIsZoom = true;
		if(!FeMath::Equal(m_dZoomRate, dRate))
		{
			m_dZoomRate = dRate;
		}
	}

	void CEarthManipulatorCtrl::StopUpdateZoom()
	{
		m_bIsZoom = false;
		m_dZoomRate = 0.0;
	}


	void CEarthManipulatorCtrl::Update()
	{
		if(m_opFreeEarthManipulator.valid())
		{
			if(m_bIsZoom)
			{
				m_opFreeEarthManipulator->zoom(1, -m_dZoomRate);
			}

			if(m_bIsRotate)
			{
				m_opFreeEarthManipulator->rotateWithCamCenter(m_dAzimuth,m_dPitch);
			}

			if(m_bIsMove)
			{
				m_opFreeEarthManipulator->panThroughCtrl(m_dX,m_dY);
			}

			if(m_opActiveCompass.valid())
			{
				osgEarth::Viewpoint point = m_opFreeEarthManipulator->getViewpoint();

				double dValue = osg::DegreesToRadians(point.getHeading());

				if(dValue < osg::PI && dValue >= -osg::PI)
				{
					m_opActiveCompass->UpdateValue(Manipulator2Navigator(dValue));
				}
			}
		}
	}

	void CEarthManipulatorCtrl::AddActiveComponent(CBasePart* pPart)
	{
		m_opActiveCompass = pPart;
	}


	double CEarthManipulatorCtrl::Navigator2Manipulator( double dValue )
	{
		if(dValue >= 0.0 && dValue < osg::PI)
		{
			dValue = osg::RadiansToDegrees(dValue);
		}
		else if(dValue >=osg::PI && dValue < PI__2)
		{
			dValue = osg::RadiansToDegrees(dValue - PI__2);
		}

		return dValue;
	}

	double CEarthManipulatorCtrl::Manipulator2Navigator( double dValue )
	{
		if(dValue >= 0.0 && dValue < osg::PI) //逆时针
		{
			return dValue;
		}
		else if(dValue < 0.0 && dValue >= -osg::PI) //顺时针
		{
			return PI__2 + dValue;
		} 

		return dValue;
	}
}
