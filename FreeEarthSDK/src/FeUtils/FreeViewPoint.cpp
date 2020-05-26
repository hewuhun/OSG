#include <FeUtils/FreeViewPoint.h>

namespace FeUtil
{


	CFreeViewPoint::CFreeViewPoint()
		:m_strName("")
		,m_dLon(0.0)
		,m_dLat(0.0)
		,m_dHeigh(0.0)
		,m_dPitch(-90.0)
		,m_dHeading(-20.0)
		,m_dRange(10000)
		,m_dTime(1.0)
	{
		
	}

	CFreeViewPoint::CFreeViewPoint( const osgEarth::Viewpoint& viewPoint )
		:m_strName(viewPoint.name().value())
		,m_dLon(viewPoint.focalPoint().value().x())
		,m_dLat(viewPoint.focalPoint().value().y())
		,m_dHeigh(viewPoint.focalPoint().value().z())
		,m_dPitch(viewPoint.getPitch())
		,m_dHeading(viewPoint.getHeading())
		,m_dRange(viewPoint.getRange())
		,m_dTime(1.0)
	{

	}

	CFreeViewPoint::CFreeViewPoint( double dLon, double dLat, double dHei, double dHeading, double dPitch, double dRange, double dTime )
		:m_strName("")
		,m_dLon(dLon)
		,m_dLat(dLat)
		,m_dHeigh(dHei)
		,m_dPitch(dPitch)
		,m_dHeading(dHeading)
		,m_dRange(dRange)
		,m_dTime(dTime)
	{

	}

	CFreeViewPoint::CFreeViewPoint( std::string strName, double dLon, double dLat, double dHei, double dHeading, double dPitch, double dRange, double dTime )
		:m_strName(strName)
		,m_dLon(dLon)
		,m_dLat(dLat)
		,m_dHeigh(dHei)
		,m_dPitch(dPitch)
		,m_dHeading(dHeading)
		,m_dRange(dRange)
		,m_dTime(dTime)
	{

	}

	CFreeViewPoint::CFreeViewPoint( const CFreeViewPoint& viewPoint )
		:m_strName(viewPoint.GetName())
		,m_dLon(viewPoint.GetLon())
		,m_dLat(viewPoint.GetLat())
		,m_dHeigh(viewPoint.GetHei())
		,m_dPitch(viewPoint.GetPitch())
		,m_dHeading(viewPoint.GetHeading())
		,m_dRange(viewPoint.GetRange())
		,m_dTime(viewPoint.GetTime())
	{

	}

	CFreeViewPoint::~CFreeViewPoint()
	{

	}


	double CFreeViewPoint::GetTime() const
	{
		return m_dTime;
	}

	osgEarth::Viewpoint CFreeViewPoint::GetEarthVP() const
	{
		osgEarth::Viewpoint vp(m_strName.c_str(), m_dLon, m_dLat, m_dHeigh, m_dHeading, m_dPitch, m_dRange);
		return vp;
	}

	void CFreeViewPoint::SetViewPoint( const osgEarth::Viewpoint& viewPoint )
	{
		m_dLon = viewPoint.focalPoint().value().x();
		m_dLat = viewPoint.focalPoint().value().y();
		m_dHeigh = viewPoint.focalPoint().value().z();
		m_dHeading = viewPoint.getHeading();
		m_dPitch = viewPoint.getPitch();
		m_dRange = viewPoint.getRange();
	}

	void CFreeViewPoint::SetViewPoint( double dLon, double dLat, double dHei, double dHeading, double dPitch, double dRange, double dTime )
	{
		m_dLon = dLon;
		m_dLat = dLat;
		m_dHeigh = dHei;
		m_dHeading = dHeading;
		m_dPitch = dPitch;
		m_dRange = dRange;
		m_dTime = dTime;
	}

	void CFreeViewPoint::SetViewPoint( const osg::Vec4d& vecFocuse )
	{
		m_dLon = vecFocuse.x();
		m_dLat = vecFocuse.y();
		m_dHeigh = vecFocuse.z();
		m_dRange = vecFocuse.w();
	}

	void CFreeViewPoint::SetTime( double dTime )
	{
		m_dTime = dTime;
	}

	double CFreeViewPoint::GetLon() const
	{
		return m_dLon;
	}

	double CFreeViewPoint::GetLat() const
	{
		return m_dLat;
	}

	double CFreeViewPoint::GetHei() const
	{
		return m_dHeigh;
	}

	double CFreeViewPoint::GetPitch() const
	{
		return m_dPitch;
	}

	double CFreeViewPoint::GetHeading() const
	{
		return m_dHeading;
	}

	double CFreeViewPoint::GetRange() const
	{
		return m_dRange;
	}

	std::string CFreeViewPoint::GetName() const
	{
		return m_strName;
	}

	void CFreeViewPoint::SetName( std::string strName )
	{
		m_strName = strName;
	}

}

namespace FeUtil
{
	CViewPointObserver::CViewPointObserver()
	{

	}

	CViewPointObserver::~CViewPointObserver()
	{

	}

	CFreeViewPoint CViewPointObserver::GetViewPoint()
	{
		return m_viewPoint;
	}

	void CViewPointObserver::SetViewPoint( const CFreeViewPoint& viewPoint )
	{
		m_viewPoint = viewPoint;
	}

}