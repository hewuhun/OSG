#ifndef CURL_HELPER_H__
#define CURL_HELPER_H__

#include <string>
#include <vector>
#include "FeServiceProvider/Export.h"
namespace FeServiceProvider
{
	class FESERVICEPROVIDER_EXPORT CurlHelper
	{
	public:
		CurlHelper();

		~CurlHelper();

		bool GetReq(const std::string & strUrl, std::string & response);
	};
}
#endif //CURL_HELPER_H__