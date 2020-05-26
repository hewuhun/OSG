#ifndef XYZ_SERVICE_PROVIDER_H__
#define XYZ_SERVICE_PROVIDER_H__

#include "ServiceProvider.h"

namespace FeServiceProvider
{
	class FESERVICEPROVIDER_EXPORT XYZServiceProvider : public ServiceProvider
	{
	public:
		XYZServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName = "", const std::string & strPassword = "");
		~XYZServiceProvider();
	public:
		std::string GetServiceType();
	};
}


#endif //XYZ_SERVICE_PROVIDER_H__