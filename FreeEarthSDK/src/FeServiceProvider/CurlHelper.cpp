#include "CurlHelper.h"

#include <curl/curl.h>
using namespace FeServiceProvider;
size_t reply(void * ptr, size_t size, size_t nmemb, void * stream)
{
	std::string * str = (std::string *)stream;
	(*str).append((char*)ptr, size * nmemb);

	return size * nmemb;
}


CurlHelper::CurlHelper()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

CurlHelper::~CurlHelper()
{
	curl_global_cleanup();
}

bool CurlHelper::GetReq(const std::string & strUrl, std::string & response)
{
	CURL * curl = curl_easy_init();
	CURLcode ret;
	if (curl == NULL)
	{
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, reply);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	ret = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (ret == CURLE_OK)
	{
		return true;
	}

	return false;
}

