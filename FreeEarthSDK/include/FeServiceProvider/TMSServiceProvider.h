#ifndef TMSServiceProvider_h__
#define TMSServiceProvider_h__

#include "ServiceProvider.h"
namespace FeServiceProvider
{
	class FESERVICEPROVIDER_EXPORT TMSServiceProvider : public ServiceProvider
	{
	public:
		TMSServiceProvider(const std::string & strName, const std::string & strUrl, const std::string & strUserName = "", const std::string & strPassword = "");
		~TMSServiceProvider();

	public:
		void GetAttr();

		virtual std::string GetServiceType();
	};
}

#endif //TMSServiceProvider_h__