#ifndef WMTS_SERVICE_PROVIDER_H__
#define WMTS_SERVICE_PROVIDER_H__

#include "ServiceProvider.h"

namespace FeServiceProvider
{
	class FESERVICEPROVIDER_EXPORT WMTSServiceProvider : public ServiceProvider
	{
	public:
		WMTSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName = "", const std::string & strPassword = "");

		~WMTSServiceProvider();

	public:
		void GetAttr();

		std::string GetServiceType();

		std::string GetServiceUrl();

	};
}


#endif //WMTS_SERVICE_PROVIDER_H__