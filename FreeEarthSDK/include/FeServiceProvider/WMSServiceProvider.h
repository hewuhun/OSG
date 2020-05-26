#ifndef WMSServiceProvider_h__
#define WMSServiceProvider_h__
#include "ServiceProvider.h"

namespace FeServiceProvider
{
	class FESERVICEPROVIDER_EXPORT WMSServiceProvider : public ServiceProvider
	{
	public:
		WMSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName = "", const std::string & strPassword = "");

		~WMSServiceProvider();

	public:
		void GetAttr();

		std::string GetServiceType();

		std::string GetServiceUrl();

		void SetVersion(const std::string & strVersion);
	};
}


#endif //WMSServiceProvider_h__