#include <FeKits/screenInfo/InfoMaker.h>

#include <osgEarthUtil/EarthManipulator>

#include <FeUtils/CoordConverter.h>
#include <FeKits/manipulator/ManipulatorUtil.h>

namespace FeKit
{
	CInfoMaker::CInfoMaker( FeUtil::CRenderContext* pContext )
		:osg::Referenced()
		,m_opRenderContext(pContext)
	{

	};

	CInfoMaker::~CInfoMaker()
	{

	}

	void CInfoMaker::GetPosture( double& dPitch, double& dHeading, double& dRange )
	{
		FeKit::CManipulatorManager* pMg = dynamic_cast<FeKit::CManipulatorManager*>(m_opRenderContext->GetView()->getCameraManipulator());
		if(pMg)
		{
			osgEarth::Viewpoint viewPoint(pMg->GetViewPoint());
			dHeading = viewPoint.getHeading();
			dPitch = viewPoint.getPitch();
			dRange = viewPoint.focalPoint().get().z();
		}
	}

}
namespace FeKit
{
	CCommonInfo::CCommonInfo( FeUtil::CRenderContext* pContext)
		:CInfoMaker(pContext)
	{

	}

	CCommonInfo::~CCommonInfo()
	{

	}

	void CCommonInfo::GetLLH( double dMouseX, double dMouseY, double& dLon, double& dLat, double& dHei )
	{
		FeUtil::ScreenXY2DegreeLLH(m_opRenderContext.get(), dMouseX, dMouseY, dLon, dLat, dHei);
	}

	void CCommonInfo::GetLLH( double dMouseX, double dMouseY, std::string& strLon, std::string& strLat, std::string& strHei )
	{
		double dLon(0.0), dLat(0.0), dHei(0.0);
		FeUtil::ScreenXY2DegreeLLH(m_opRenderContext.get(), dMouseX, dMouseY, dLon, dLat, dHei);

		char cTemp[100];
		std::sprintf(cTemp, "经  度: %.6f°", dLon);
		strLon = cTemp;

		std::sprintf(cTemp, "纬  度: %0.6f°", dLat);
		strLat = cTemp;
	
		std::sprintf(cTemp, "海  拔: %.3f", dHei);
		strHei = cTemp;
	}

}

namespace FeKit
{
	CArmyInfo::CArmyInfo( FeUtil::CRenderContext* pContext )
		:CInfoMaker(pContext)
	{

	}

	CArmyInfo::~CArmyInfo()
	{

	}

	void CArmyInfo::GetLLH( double dMouseX, double dMouseY, double& dLon, double& dLat, double& dHei )
	{
		FeUtil::ScreenXY2DegreeLLH(m_opRenderContext.get(), dMouseX, dMouseY, dLon, dLat, dHei);
	}

	void CArmyInfo::GetLLH( double dMouseX, double dMouseY, std::string& strLon, std::string& strLat, std::string& strHei )
	{
		double dLon(0.0), dLat(0.0), dHei(0.0), X(0.0), Y(0.0);
		FeUtil::ScreenXY2DegreeLLH(m_opRenderContext.get(), dMouseX, dMouseY, dLon, dLat, dHei);
	//	FeUtil::Eaadsp_ProjectionFunction::Conic_to_XY_OLD(dLat, dLon, &X, &Y);

		std::stringstream osLon;
		osLon<<"横坐标: "<<std::setiosflags(std::ios::fixed)<<std::setprecision(3)<<X;
		strLon = osLon.str();

		std::stringstream osLat;
		osLat<<"纵坐标: "<<std::setiosflags(std::ios::fixed)<<std::setprecision(3)<<Y;
		strLat = osLat.str();

		std::stringstream osHei;
		osHei<<"海  拔: "<<std::setiosflags(std::ios::fixed)<<std::setprecision(3)<<dHei;
		strHei = osHei.str();
	}


}