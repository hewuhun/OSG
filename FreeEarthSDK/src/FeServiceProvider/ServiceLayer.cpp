
#include "FeServiceProvider/ServiceLayer.h"
#include "FeServiceProvider/ServiceProvider.h"
using namespace FeServiceProvider;

ServiceLayer::ServiceLayer()
{
	m_strEPSGCode = "";
	m_strCRSCode = "";
	m_strName = "";
	
	m_strLower = "";
	m_strUpper = "";
}

ServiceLayer::~ServiceLayer()
{

}

std::string ServiceLayer::GetEPSGCode()
{
	return m_strEPSGCode;
}

std::string ServiceLayer::GetCRSCode()
{
	return m_strCRSCode;
}

std::string ServiceLayer::GetName()
{
	return m_strName;
}



std::string ServiceLayer::GetLowerCorner()
{
	return m_strLower;
}

std::string ServiceLayer::GetUpperCorner()
{
	return m_strUpper;
}

ServiceProvider * ServiceLayer::GetServiceProvider()
{
	return m_pServiceProvider;
}

void ServiceLayer::SetEPSGCode(const std::string & code)
{
	m_strEPSGCode = code;
}

void ServiceLayer::SetCRSCode(const std::string & code)
{
	m_strCRSCode = code;
}

void ServiceLayer::SetName(const std::string & name)
{
	m_strName = name;
}

void ServiceLayer::SetLowerCorner(const std::string & lower)
{
	m_strLower = lower;
}

void ServiceLayer::SetUpperCorner(const std::string & upper)
{
	m_strUpper = upper;
}

void ServiceLayer::SetServiceProvider(ServiceProvider * serviceProvider)
{
	m_pServiceProvider = serviceProvider;
}