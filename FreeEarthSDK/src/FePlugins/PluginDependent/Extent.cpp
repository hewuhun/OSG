#include <PluginDependent/Extent.h>

namespace FePlugin
{
	CExtent::CExtent() 
		:m_bIsValid(false)
	{

	}

	CExtent::CExtent( double dXMin, double dYMin, double dXMax, double dYMax, const std::string& strSrs )
		:m_dXMin(dXMin), 
		m_dYMin(dYMin), 
		m_dXMax(dXMax),
		m_dYMax(dYMax),
		m_strSrsName(strSrs),
		m_bIsValid(true)
	{

	}

	CExtent::CExtent( const CExtent& rhs )
		: m_dXMin(rhs.m_dXMin), 
		m_dYMin(rhs.m_dYMin),
		m_dXMax(rhs.m_dXMax), 
		m_dYMax(rhs.m_dYMax),
		m_strSrsName(rhs.m_strSrsName),
		m_bIsValid(rhs.m_bIsValid)
	{

	}

	CExtent & CExtent::operator= (const CExtent &other)
	{
		if (&other == this)
		{
			return *this;
		}
		else
		{
			this->m_bIsValid = other.m_bIsValid;
			this->m_dXMax = other.m_dXMax;
			this->m_dXMin = other.m_dXMin;
			this->m_dYMax = other.m_dYMax;
			this->m_dYMin = other.m_dYMin;
			this->m_strSrsName = other.m_strSrsName;

			return *this;
		}
	}

	CExtent::~CExtent()
	{

	}
}


