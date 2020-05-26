#ifndef WCSServiceProvider_h__
#define WCSServiceProvider_h__

#include "ServiceProvider.h"
namespace FeServiceProvider
{
	class FESERVICEPROVIDER_EXPORT WCSServiceProvider : public ServiceProvider
	{
	public:
		WCSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName = "", const std::string & strPassword = "");

		~WCSServiceProvider();
	public:
		void GetAttr();

		std::string GetServiceType();

		std::string GetServiceUrl();

		void SetVersion(const std::string & strVersion);
	};
}

#endif //WCSServiceProvider