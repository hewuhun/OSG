#ifndef WFS_SERVICE_PROVIDER_H__
#define WFS_SERVICE_PROVIDER_H__
#include "ServiceProvider.h"
namespace FeServiceProvider
{
	class FESERVICEPROVIDER_EXPORT WFSServiceProvider : public ServiceProvider
	{
	public:
		WFSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName = "", const std::string & strPassword = "");
		~WFSServiceProvider();
	public:
		void GetAttr();

		std::string GetServiceType();

		std::string GetServiceUrl();

		void SetVersion(const std::string & strVersion);
	};
}

#endif //WFS_SERVICE_PROVIDER_H__
