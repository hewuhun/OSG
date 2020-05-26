#ifndef DATA_SERVICE_PROVIDER_H__
#define DATA_SERVICE_PROVIDER_H__

#include "ServiceProvider.h"
namespace FeServiceProvider
{
	class  FESERVICEPROVIDER_EXPORT LocalDataServiceProvider : public ServiceProvider
	{
	public:
		LocalDataServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & plugin = "flatgis");

		~LocalDataServiceProvider();
	public:
		std::string GetPluginType();

		virtual std::string GetServiceType();

		virtual void SetPluginType(const std::string & strPluginType);
	};
}

#endif //DATA_SERVICE_PROVIDER_H__
